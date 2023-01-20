use yaml_rust::{YamlLoader, Yaml};
use yaml_rust::yaml::Hash;
//use jsonschema::JSONSchema;

fn string_to_json(s: &String) -> String {
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

fn array_to_json(a: &Vec<Yaml>) -> String {
    let mut vec = Vec::new();

    for val in a {
        vec.push(to_json(val));
    }

    format!("[{}]", vec.join(",")).to_string()
}

fn hash_to_json(h: &Hash) -> String {
    let mut vec = Vec::new();

    for (key, val) in h {
        vec.push(format!("{}:{}", to_json(key), to_json(val)));
    }

    format!("{{{}}}", vec.join(",")).to_string()
}

fn to_json(val: &Yaml) -> String {
    let ret;

    match &*val {
        Yaml::Real(s) => {
            ret = s.clone();
        },

        Yaml::Integer(i) => {
            ret = format!("{}", i).to_string();
        },

        Yaml::String(s) => {
            ret = string_to_json(s);
        },

        Yaml::Boolean(b) => {
            ret = String::from(if *b {"true"} else {"false"});
        },

        Yaml::Array(a) => {
            ret = array_to_json(a);
        },

        Yaml::Hash(h) => {
            ret = hash_to_json(h);
        },

        Yaml::Null => {
            ret = "null".to_string();
        },

        _ => {
            ret = "".to_string();
        },
    }

    return ret;
}

fn main() {
    let yaml = include_str!("../data/sample.yml");
    let doc  = YamlLoader::load_from_str(&yaml).unwrap();

    println!("{}", to_json(&doc[0]));
}
