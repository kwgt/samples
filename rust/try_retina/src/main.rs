/*
 * use example for retina (https://github.com/scottlamb/retina)
 *
 *  Copyright (C) 2023 Hiroshi Kuwagata <kgt9221@gmail.com>
 */

use tokio::time::sleep;
use std::time::Duration;

pub mod rtsp {
    //! RTSPの受信を行う処理をまとめたモジュール

    use anyhow::{anyhow, Result};
    use url::Url;
    use futures::StreamExt;
    use retina::client::*;
    use retina::codec::*;
    use h264_reader::avcc::AvcDecoderConfigurationRecord;
    use tokio::sync::watch;
    use tokio::sync::watch::Sender;
    use tokio::task::JoinHandle;

    /// 指定された文字列をURLオブジェクトに変換する関数
    ///
    /// # 引数
    /// * `url` - 変換元の文字列
    ///
    /// # 戻り値
    /// URLオブジェクト (url::Url)
    ///
    /// # 注記
    /// 本関数はエラー制御の統一する目的だけで実装されている
    fn parse_url(url: &str) -> Result<Url> {
        match Url::parse(url) {
            Ok(url) => Ok(url),
            Err(error) => Err(anyhow!(error.to_string())),
        }
    }

    /// セッションオプションを生成する関数
    ///
    /// # 引数
    /// `creds` - 認証情報
    ///
    /// # 戻り値
    /// セッションオプションをパックしたオブジェクト
    fn session_opts(creds: &Option<Credentials>)
        -> SessionOptions
    {
        SessionOptions::default()
            .creds(creds.clone())
            .teardown(TeardownPolicy::Always)
    }

    /// セットアップオプションを生成する関数
    ///
    /// # 戻り値
    /// セットアップオプションをパックしたオブジェクト
    fn setup_opts() -> SetupOptions {
        SetupOptions::default()
            .transport(Transport::Tcp(TcpTransportOptions::default()))
    }

    /// プレイオプションをパックしたオブジェクト
    ///
    /// # 戻り値
    /// プレイオプションをパックしたオブジェクト
    fn play_opts() -> PlayOptions {
        PlayOptions::default()
            .ignore_zero_seq(true)
    }

    /// ストリームインデックスの探査を行う関数
    ///
    /// # 引数
    /// `media` - 探査対象のメディア指定子("video"など)
    /// `coding` - 探査対象の符号化方法指定子("H264"など)
    ///
    /// # 戻り値
    /// 見つかった場合はSome()で見つかったストリームのインデックスを返す。見つ
    /// からなかった場合はNoneを返す。
    ///
    /// # 注記
    /// 本関数はSDPの探査と同等の処理を行っている
    fn stream_index(session: &Session<Described>, media: &str, encoding: &str)
        -> Option<usize>
    {
        session.streams().iter().position(|s| {
            s.media() == media && s.encoding_name() == encoding
        })
    }

    /// デバッグ用関数
    ///
    /// # 注記
    /// SPSとPPSをストリーム情報をもとに探し出して表示している
    fn dump_stream(s: &Stream) {
        if let Some(params) = s.parameters() {
            if let ParametersRef::Video(params) = params {

                if s.encoding_name() == "h264" {
                    let avcc = AvcDecoderConfigurationRecord::try_from(params.extra_data()).unwrap();
                    for sps in avcc.sequence_parameter_sets() {
                        println!("SPS {:?}", sps);
                    }

                    for pps in avcc.picture_parameter_sets() {
                        println!("PPS {:?}", pps);
                    }
                }
            }
        }
    }

    /// RTSPのセッション状態指定子の定義
    enum SessionState {
        NoConnection,
        Described,
        BeginPlay,
        Playing,
        Teardown,
    }

    impl SessionState {
        /// 接続中か否かを判定する関数
        ///
        /// # 戻り値
        /// サーバと接続中である場合はtrueを返す
        #[allow(dead_code)]
        fn is_connected(&self) -> bool {
            self.is_described() || self.is_playing()
        }

        /// DESCRIBEが完了しているかを判定する関数
        ///
        /// # 戻り値
        /// DESCRIBEが完了している場合はtrueを返す
        #[allow(dead_code)]
        fn is_described(&self) -> bool {
            match self {
                SessionState::Described => true,
                _ => false,
            }
        }

        /// 再生中か否かを判定する関数
        ///
        /// # 戻り値
        /// 再生中の場合はtrueを返す
        #[allow(dead_code)]
        fn is_playing(&self) -> bool {
            match self {
                SessionState::BeginPlay => true,
                SessionState::Playing => true,
                _ => false,
            }
        }

        /// TEARDOWNが行われたか否かを判定する関数
        ///
        /// # 戻り値
        /// TEARDOWN済みの場合はtrueを返す
        #[allow(dead_code)]
        fn is_teardown(&self) -> bool {
            match self {
                SessionState::Teardown => true,
                _ => false,
            }
        }
    }

    /// RTSP受信処理を抽象化した構造体
    pub struct RtspReceiver {
        /// 接続先のURL
        url: String,

        /// 接続時に用いる認証情報
        /// 認証を行う場合のみSome, 認証を行わない（必要がない）カメラに接続す
        /// る場合はNone
        creds: Option<Credentials>,

        /// RTSPセッションの状態
        state: SessionState,

        /// DESCRIBE後からPLAYまでの期間に参照するためのセッション情報
        /// SessionStateがDescribedの期間のみSome、それ以外はNone
        session: Option<Session<Described>>,

        /// 受信終了を受信処理(async)に通知するための通信経路
        /// SessionStateがPlayingの期間のみSome, それ以外はNone
        channel: Option<Sender<bool>>,

        /// 受信終了時に受信処理(async)の終了待ち合わせを行うためのハンドル
        /// SessionStateがPlayingの期間のみSome, それ以外はNone
        handle: Option<JoinHandle<()>>,
    }

    impl RtspReceiver {
        /// コンストラクター(認証情報なし)
        ///
        /// # 引数
        /// `url` - 接続先のURL
        ///
        /// # 戻り値
        /// RTSP受信処理オブジェクト
        ///
        /// # 注記
        /// 認証が必要なカメラに接続する場合は new_with_creds()を使用すること。
        #[allow(dead_code)]
        pub fn new(url: &str) -> Self {
            RtspReceiver {
                url: String::from(url),
                creds: None,
                state: SessionState::NoConnection,
                session: None,
                channel: None,
                handle: None,
            }
        }

        /// コンストラクター(認証情報あり)
        ///
        /// # 引数
        /// `url` - 接続先のURL
        /// `username` - 認証情報(ユーザ名)
        /// `password` - 認証情報(パスワード)
        ///
        /// # 戻り値
        /// RTSP受信処理オブジェクト
        ///
        /// # 注記
        /// 認証が必要無いカメラに接続する場合はnew()を使用すること。
        #[allow(dead_code)]
        pub fn new_with_creds(url: &str, username: &str, password: &str)
            -> Self
        {
            let creds = Credentials {
                username: String::from(username),
                password: String::from(password),
            };

            RtspReceiver {
                url: String::from(url),
                creds: Some(creds),
                state: SessionState::NoConnection,
                session: None,
                channel: None,
                handle: None,
            }
        }

        /// カメラへの接続とDESCRIBEの発行
        ///
        /// # 戻り値
        /// 接続を試みた結果を返す。成功した場合はOk(())を返す。失敗した場合は
        /// Err(err)を返す。
        ///
        /// # 注記
        /// 本関数は未接続時のみ実行できる
        pub async fn connect(&mut self) -> Result<()>
        {
            if self.state.is_connected() {
                return Err(anyhow!("already connected"));
            }

            let url = parse_url(&self.url)?;
            let opts = session_opts(&self.creds);
            let session = Session::describe(url, opts).await?;

            for stream in session.streams() {
                dump_stream(&stream);
            }

            self.state = SessionState::Described;
            self.session = Some(session);

            Ok(())
        }

        /// SETUPの発行（受信ストリーム設定）
        ///
        /// # 戻り値
        /// SETUPを試みた結果を返す。成功した場合はOk(())を返す。失敗した場合は
        /// Err(err)を返す。
        ///
        /// # 注記
        /// 本関数はDESCRIBE発行後〜PLAY発行までの期間にのみ実行できる。
        pub async fn setup(&mut self, media: &str, encoding: &str)
            -> Result<()>
        {
            if !self.state.is_described() {
                return Err(anyhow!("not described state"));
            }

            let session = self.session.as_mut().unwrap();
            if let Some(idx) = stream_index(&session, media, encoding) {
                session.setup(idx, setup_opts()).await?;

            } else {
                return Err(anyhow!("target not exist"));
            }

            Ok(())
        }

        /// PLAYの発行（再生の開始）
        ///
        /// # 戻り値
        /// PLAYを試みた結果を返す。成功した場合はOk(())を返す。失敗した場合は
        /// Err(err)を返す。
        ///
        /// # 注記
        /// 本関数はDESCRIBE発行後にのみ実行できる。本関数を発行するとPLAY発行
        /// 状態に遷移するのでSETUPは発行不可となる注意すること。
        /// また本関数の呼び出しに成功すると、カメラ側からデータ送信が行われる。
        pub async fn play(&mut self) -> Result<()> {
            if !self.state.is_described() {
                return Err(anyhow!("not described state"));
            }

            self.state = SessionState::BeginPlay;
            let session = self.session.take().unwrap();

            let res = session.play(play_opts()).await;

            if let Err(error) = res {
                return Err(anyhow!(error.to_string()));
            }

            let session = res.unwrap();
            let mut demuxer = session.demuxed()?;

            let (tx, mut rx) = watch::channel::<bool>(false);

            let handle = tokio::spawn(async move {
                loop {
                    tokio::select! {
                        data = demuxer.next() => {
                            if data.is_none() {
                                continue;
                            }

                            let data = data.unwrap();
                            if data.is_err() {
                                continue;
                            }

                            if let CodecItem::VideoFrame(v) = data.unwrap() {
                              println!("{}", v.timestamp().elapsed_secs());
                            }
                        }

                        res = rx.changed() => {
                            if let Ok(_) = res {
                                break;
                            }
                        }
                    }
                }
            });

            self.state = SessionState::Playing;
            self.channel = Some(tx);
            self.handle = Some(handle);

            Ok(())
        }

        /// STOPの発行(再生の停止)
        ///
        /// # 戻り値
        /// TEARDOWNを試みた結果を返す。成功した場合はOk(())を返す。失敗した場
        /// 合はErr(err)を返す。
        ///
        /// # 注記
        /// 本関数はPLAY発行後にのみ実行できる。本関数を発行するとTEARDOWN発行
        /// 状態に遷移する。
        /// 本関数の呼び出しに成功すると、カメラ側からのデータ送信が停止する。
        pub async fn stop(&mut self) -> Result<()> {
            if !self.state.is_playing() {
                return Err(anyhow!("not playing yet"));
            }

            self.channel.as_mut().unwrap().send(true)?;
            self.handle.as_mut().unwrap().await?;

            self.state = SessionState::Teardown;
            self.channel = None;
            self.handle = None;

            Ok(())
        }
    }
}

#[tokio::main]
async fn main() {
    let url = "rtsp://172.27.0.90:554/unicast/codec1";
    let username = "admin";
    let password = "guest";

    let mut rtsp = rtsp::RtspReceiver::new_with_creds(url, username, password);

    rtsp.connect().await;
    rtsp.setup("video", "h264").await;
    rtsp.play().await;

    sleep(Duration::from_secs(3600)).await;
    rtsp.stop().await;
}
