#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ALLOC(t)          ((t*)malloc(sizeof(t)))
#define NALLOC(t,n)       ((t*)malloc(sizeof(t)*(n)))

#define DEFAULT_ERROR     __LINE__

static int
dig(char* path, int mode)
{
  int ret;
  int err;
  struct stat st;

  ret = 0;

  err = stat(path, &st);
  if (!err) {
    if (!S_ISDIR(st.st_mode)) ret = DEFAULT_ERROR;

  } else {
    if (errno == ENOENT) {
      err = mkdir(path, mode);
      if (err) ret = DEFAULT_ERROR;

    } else {
      ret = DEFAULT_ERROR;
    }
  }

  return ret;
}

typedef struct {
  uid_t uid;
  gid_t* list;
  int n;
} owner_info_t;

#define MAX_DEPTH       40

typedef struct {
  int abs;
  char** list;
  size_t depth;
} pathname_t;

static int
pathname_new(char* src, pathname_t** dst)
{
  int ret;
  pathname_t* obj;
  int abs;
  char* p0;
  int len;
  char* sub;
  char* list[MAX_DEPTH];
  size_t dep;
  int st;
  int i;

  /*
   * initialize
   */
  ret = 0;
  obj = NULL;
  abs  = 0;
  sub  = NULL;
  dep  = 0;
  st   = 0;

  /*
   * argument check
   */
  do {
    if (src == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    if (dst == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }
  } while (0);

  /*
   * memory allocate
   */
  if (!ret) {
    obj = ALLOC(pathname_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * split path string
   */
  if (!ret) {
    do {
      switch (st) {
      case 0:     /* 初期状態 */
        switch (*src) {
        case '/':
          // 連続するパスセパレータの除去のためのbreak
          break;

        case '\0':
          // この状態で解析文字列終端に達した場合は追加する情報がない
          continue;

        default:
          p0 = src;
          st = 1;
          break;
        }
        break;

      case 1:   /* 通常状態 */
        if (*src == '/' || *src == '\0') {
          // サブパス文字列の切り出し
          len = src - p0;
          sub = malloc(len + 1);
          if (sub == NULL) {
            ret = DEFAULT_ERROR;
            continue;
          }

          memcpy(sub, p0, len);
          sub[len] = '\0';

          // サブパスが"."なら無視
          if (!strcmp(sub, ".")) {
            free(sub);
            st = 0;
            break;
          }

          // サブパスが".."ならリストの末端を捨てる
          // ただし消せるものが無ければエラー
          if (!strcmp(sub, "..")) {
            free(sub);

            if (dep == 0) {
              ret = DEFAULT_ERROR;
              continue;

            } else {
              free(list[dep--]);
              st = 0;
              break;
            }
          }

          // この時点でリストに空きが無ければエラー
          if (dep == MAX_DEPTH) {
            ret = DEFAULT_ERROR;
            free(sub);
            continue;
          }

          // サブパスリストにサブパスを追加
          list[dep++] = sub;
          st = 0;
        }
        break;

      default:
        abort();
      }

      src++;
    } while (!ret && *src != '\0');
  }

  /*
   * setup object
   */
  if (!ret) {
    obj->abs   = abs;
    obj->list  = list;
    obj->depth = dep;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = obj;

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
    for (i = 0; i < dep; i++) free(list[i]);
  }

  return ret;
}

static int
pathname_join(pathname_t* ptr, char** dst)
{
}

static int
pathname_destroy(pathname_t* ptr)
{
  int ret;
  int i;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release memory
   */
  if (!ret) {
    for (i = 0; i < ptr->depth; i++) free(ptr->list[i]);
    free(ptr);
  }

  return ret;
}

static int
path_canonicalize(char* src, char** dst)
{
  int ret;
  int err;
  pathname_t* pat;

  ret = 0;
  pat = NULL;

  do {
    err = pathname_new(src, &pat);
    if (err) {
      ret = DEFAULT_ERROR;
      break;
    }

    fprintf(stderr, "%p\n", pat);
  } while (0);

  if (pat != NULL) pathname_destroy(pat);

  return ret;
}

static int
owner_info_new(owner_info_t** dst)
{
  int ret;
  struct passwd* pw;
  owner_info_t* obj;
  uid_t uid;
  gid_t* list;
  int n;

  /*
   * initialize
   */
  ret  = 0;
  obj  = NULL;
  uid  = geteuid();
  list = NULL;
  n    = 0;

  /*
   * argument check
   */
  if (dst == NULL) ret = DEFAULT_ERROR;

  /*
   * get passwd entry info
   */
  if (!ret) {
    pw = getpwuid(uid);
  }

  /*
   * memory allocate
   */
  if (!ret) {
    obj = ALLOC(owner_info_t);
    if (obj == NULL) ret = DEFAULT_ERROR;
  }

  if (!ret) {
    getgrouplist(pw->pw_name, pw->pw_gid, NULL, &n);

    list = NALLOC(gid_t, n);
    if (list == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * get group list
   */
  if (!ret) {
    getgrouplist(pw->pw_name, pw->pw_gid, list, &n);
  }

  /*
   * setup objct
   */
  if (!ret) {
    obj->uid  = uid;
    obj->list = list;
    obj->n    = n;
  }

  /*
   * put return parameter
   */
  if (!ret) *dst = obj;

  /*
   * post process
   */
  if (ret) {
    if (obj != NULL) free(obj);
    if (list != NULL) free(list);
  }

  return ret;
}

static int
owner_info_destroy(owner_info_t* ptr)
{
  int ret;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * release 
   */
  if (!ret) {
    if (ptr->list != NULL) free(ptr->list);
    free(ptr);
  }

  return ret;
}

static int
owner_info_check(owner_info_t* ptr, gid_t gid)
{
  int ret;
  int i;

  /*
   * initialize
   */
  ret = 0;

  /*
   * argument check
   */
  if (ptr == NULL) ret = DEFAULT_ERROR;

  /*
   * check gid
   */
  if (!ret) {
    for (i = 0; i < ptr->n; i++) if (ptr->list[i] == gid) break;
    if (i == ptr->n) ret = DEFAULT_ERROR;
  }

  return ret;
}
static int
check_removable(char* path, owner_info_t* gl)
{
  int ret;
  int err;
  int wen;      // as "Write ENable"
  struct stat st;

  char* sub;
  char* tmp;
  size_t len;

  DIR* dir;
  struct dirent* ent;

  /*
   * initialize
   */
  ret  = 0;
  dir  = NULL;
  sub  = NULL;

  /*
   * main process (do it check for writable)
   */
  do {
    err = lstat(path, &st);
    if (err) {
      ret = DEFAULT_ERROR;
      break;
    }

     // ディレクトリの場合はエントリを再帰的にチェック
    if (S_ISDIR(st.st_mode)) {
      dir = opendir(path);
      if (dir == NULL) {
        ret = DEFAULT_ERROR;
        break;
      }

      len = strlen(path);

      do {
        ent = readdir(dir);
        if (ent == NULL) break;
        if (!strcmp(ent->d_name, ".")) continue;
        if (!strcmp(ent->d_name, "..")) continue;

        tmp = (char*)realloc(sub, len + strlen(ent->d_name) + 2);
        if (tmp != NULL) {
          sprintf(tmp, "%s/%s", path, ent->d_name);
          sub = tmp;
          ret = check_removable(sub, gl);

        } else {
          ret = DEFAULT_ERROR;
        }
      } while (!ret);
    }
    
    if (!ret) {
      if ((st.st_mode & S_IWUSR) && (st.st_uid == gl->uid)) break;
      if ((st.st_mode & S_IWGRP) && !owner_info_check(gl, st.st_gid)) break;

      // ここに到達した場合は権限不足で削除不可
      ret = DEFAULT_ERROR;
    }
  } while (0);

  /*
   * post process
   */
  if (sub != NULL) free(sub);
  if (dir != NULL) closedir(dir);

  return ret;
}

static int
remove_entry(char* path)
{
  int ret;
  int err;
  struct stat st;

  char* sub;
  char* tmp;
  size_t len;

  DIR* dir;
  struct dirent* ent;

  /*
   * initialize
   */
  ret = 0;
  dir = NULL;
  sub = NULL;

  do {
    // とりあえずunlink()してみて成功なら終了。
    err = unlink(path);
    if (!err) break;

    // unlink()して消せなかったらディレクトリかもしれないので
    // ディレクトリとして開いてみる
    dir = opendir(path);
    if (dir == NULL) {
      ret = DEFAULT_ERROR;
      break;
    }

    // ディレクトリとしてアクセスできたらディレクトリエントリごとに削除を
    // 試みる
    len = strlen(path);

    do {
      ent = readdir(dir);
      if (ent == NULL) break;
      if (!strcmp(ent->d_name, ".")) continue;
      if (!strcmp(ent->d_name, "..")) continue;

      tmp = (char*)realloc(sub, len + strlen(ent->d_name) + 2);
      if (tmp != NULL) {
        sprintf(tmp, "%s/%s", path, ent->d_name);
        sub = tmp;
        ret = remove_entry(sub);

      } else {
        ret = DEFAULT_ERROR;
      }
    } while (!ret);

    // 最後に指定されたパスをディレクトリとして削除
    if (!ret) {
      err = rmdir(path);
      if (err) {
        ret = DEFAULT_ERROR;
        break;
      }
    }
  } while (0);

  /*
   * post process
   */
  if (sub != NULL) free(sub);
  if (dir != NULL) closedir(dir);

  return ret;
}

/**
 * ディレクトリパスの作成
 *
 * @param _path  ディレクトリパスを表す文字列
 * @param mode   作成したディレクトリに設定するモードマスク
 *
 * @return エラーコード(0で正常終了)
 *
 * @note
 *  mkdir(2)と異なり指定されたパスのサブパスが存在しない場合でも可能な限り
 *  ディレクトリを作成する。
 *
 * @note
 *  指定されたパスでディレクトリが既に存在する場合はエラーとはならない。
 */
int
mkpath(char* _path, int mode)
{
  int ret;
  int err;
  char* path;
  char* p0;
  char* p1;
  char* p2;

  /*
   * initialize
   */
  ret  = 0;
  path = NULL;
  p0   = NULL;
  p1   = NULL;
  p2   = NULL;

  /*
   * argument check
   */
  if (_path == NULL) ret = DEFAULT_ERROR;

  /*
   * path regulize
   */
  //if (!ret) {
  //  err = path_canonicalize(_path, NULL);
  //  if (err) ret = DEFAULT_ERROR;
  //}

  /*
   * duplicate target path string for temperary use.
   */
  if (!ret) {
    path = malloc(strlen(_path));
    if (path == NULL) ret = DEFAULT_ERROR;
  }

  /*
   * create directory tree
   */
  if (!ret) {
    p0 = _path;
    p2 = path;

    if (*p0 == '/') {
      // 絶対パス指定の場合
      *p2++ = '/';
      p0++;

    } else {
      // 相対パス指定の場合
      p2[0] = '\0';
    }

    while (1) {
      if ((p1 = strchr(p0, '/')) != NULL) {
        memcpy(p2, p0, p1 - p0);
        p2 += (p1 - p0);
        *p2 = '\0';

      } else {
        strcpy(p2, p0);
      }

      // printf("%s\n", path);
      err = dig(path, mode);
      if (err) {
        ret = DEFAULT_ERROR;
        break;
      }

      if (p1 == NULL) break;

      *p2++ = '/';
      p0 = p1 + 1;
    }
  }

  /*
   * post process
   */
  //if (_path != NULL) free(_path);
  if (path != NULL) free(path);

  return ret;
}

/**
 * ディレクトリツリーの削除
 *
 * @param _path  削除するディレクトリのパス
 *
 * @return エラーコード(0で正常終了)
 *
 * @note
 *  rmdir(2)と異なり指定されたパス以下のツリーを再帰的に削除する（空ではない
 *  ディレクトリを指定できる）。
 *
 * @note
 *  ファイルを指定しても削除する。
 *
 * @note
 *  権限不足でツリーに削除できないエントリが含まれる場合は、エラーとなる。こ
 *  の場合は、ツリーの状態を可能な限り保全する（中途半端に一部のファイルが削
 *  除されたような状態にはしない）。
 */

int
rmtree(char* path)
{
  int ret;
  int err;
  owner_info_t* gl;

  /*
   * initialize
   */
  ret = 0;
  gl  = NULL;

  /*
   * argument check
   */
  if (path == NULL) ret = DEFAULT_ERROR;

  /*
   * path regulize
   */
  //if (!ret) {
  //  err = path_canonicalize(path, NULL);
  //  if (err) ret = DEFAULT_ERROR;
  //}

  /*
   * get group list
   */
  if (!ret) {
    err = owner_info_new(&gl);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * check removability
   */
  if (!ret) {
    err = check_removable(path, gl);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * remove file entry
   */
  if (!ret) {
    err = remove_entry(path);
    if (err) ret = DEFAULT_ERROR;
  }

  /*
   * post process
   */
  //if (path != NULL) free(path);
  if (gl != NULL) owner_info_destroy(gl);

  return ret;
}

#if 1
int
main(int argc, char* argv[])
{
  int err;

  /*

  err = mkpath("/home/kgtr/dlp/", 0777);
  printf("%d\n",err);

  err = mkpath("aaa/bb/cc", 0777);
  printf("%d\n",err);

  err = mkpath("./aaa/BB/CC", 0777);
  printf("%d\n",err);

  err = mkpath("./aaa/BB/DD/", 0777);
  printf("%d\n",err);

  err = mkpath("aaa/bb/cc", 0777);
  printf("%d\n",err);

  err = mkpath("./aaa/BB/CC", 0777);
  printf("%d\n",err);

  err = mkpath("./aaa/BB/DD/", 0777);
  printf("%d\n",err);
  */

  err = rmtree("./aaa");
  printf("%d\n",err);

  //err = path_canonicalize("./aaa/BB/DD/", NULL);
}
#endif
