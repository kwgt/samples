use serde_yaml::Value;

fn str_to_json(s: &String) -> String {
    let mut ret = String::new();

    ret.push('"');

    for ch in s.chars() {
        match ch {
            '\u{0000}' => {
                // NUL
                ret.push_str("\\0");
            },
            '\u{0009}' => {
                // HT
                ret.push_str("\\t");
            },
            '\u{000a}' => {
                // LF
                ret.push_str("\\n");
            },
            '\u{000b}' => {
                // VT
                ret.push_str("\\v");
            },
            '\u{000c}' => {
                // FF
                ret.push_str("\\f");
            },
            '\u{000d}' => {
                // CR
                ret.push_str("\\r");
            },
            '\u{0022}' => {
                // double quote
                ret.push_str("\\\"");
            },
            '\u{0027}' => {
                // single quote
                ret.push_str("\\'");
            },
            '\u{005c}' => {
                // back slash
                ret.push_str("\\\\");
            },
            '\u{0060}' => {
                // back quote
                ret.push_str("\\`");
            },
            _ => {
                ret.push(ch)
            },
        };
    }

    ret.push('"');

    return ret;
}

fn seq_to_json(s: &Vec<Value>) -> String {
    let mut vec = Vec::new();

    for v in s {
        vec.push(val_to_json(v));
    }

    format!("[{}]", vec.join(",")).to_string()
}

fn map_to_json(h: &serde_yaml::Mapping) -> String {
    let mut vec = Vec::new();

    for (k, v) in h {
        vec.push(format!("{}:{}", val_to_json(k), val_to_json(v)));
    }

    format!("{{{}}}", vec.join(",")).to_string()
}

fn val_to_json(v: &Value) -> String {
    let ret;

    match &*v {
        Value::Null => {
            ret = "null".to_string();
        },

        Value::Bool(b) => {
            ret = (if *b {"true"} else {"false"}).to_string();
        },

        Value::Number(n) if n.is_i64() => {
            ret = n.to_string();
        },

        Value::Number(n) if n.is_u64() => {
            ret = n.to_string();
        },

        Value::Number(n) if n.is_f64() => {
            ret = n.to_string();
        },

        Value::String(s) => {
            ret = str_to_json(&s);
        },

        Value::Sequence(s) => {
            ret = seq_to_json(&s);
        },

        Value::Mapping(m) => {
            ret = map_to_json(&m);
        },
        _ => {
            ret = "bad value".to_string();
        },
    }

    return ret;
}

fn main() {
    let yaml = include_str!("../data/sample.yml");
    let doc: Value = serde_yaml::from_str(&yaml).unwrap();

    println!("{}", val_to_json(&doc));
}
