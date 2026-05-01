#!/usr/bin/env python3
import os
import re
import json
import argparse

def get_project_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def parse_enum_comments(filepath):
    """Parse a header file and extract enum values and their trailing comments."""
    result = {}
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the enum block
    enum_match = re.search(r'enum\s+class\s+\w+[^\{]*\{([^}]+)\}', content)
    if not enum_match:
        return result

    enum_body = enum_match.group(1)
    # Match lines like: key = value, // comment
    # or just: key, // comment
    for line in enum_body.split('\n'):
        line = line.strip()
        if not line or line.startswith('//'):
            continue
        
        # Match key and comment
        m = re.match(r'^([a-zA-Z0-9_]+).*?//\s*(.*)$', line)
        if m:
            key = m.group(1)
            comment = m.group(2).strip()
            # Clean up the key if it has trailing underscore (like namespace_)
            clean_key = key
            result[clean_key] = comment

    return result

def to_english_title(key):
    """Convert snake_case key to Title Case (e.g. rule_engine -> Rule Engine)"""
    if key == "none":
        return "None"
    # Special cases
    if key.endswith('_'):
        key = key[:-1]
    
    words = key.split('_')
    # Capitalize acronyms if known, else title case
    special_acronyms = {'api', 'rpc', 'http', 'https', 'tcp', 'udp', 'dns', 'ssl', 'tls', 'ftp', 'smtp', 'icmp', 'quic', 'sctp', 'mqtt', 'coap', 'ntp', 'vpn', 'sdn', 'cdn', 'aws', 'oss', 'cos', 'hdfs', 'nfs', 'ceph', 'minio', 'gcs', 'san', 'nas', 'sql', 'mysql', 'mongodb', 'postgresql', 'sqlite', 'tidb', 'oracle', 'sqlserver', 'mariadb', 'neo4j', 'influxdb', 'etcd', 'sso', 'mfa', 'oauth', 'ldap', 'hsm', 'siem', 'waf', 'ddos', 'llm', 'nlp', 'ocr', 'ai', 'ar', 'vr', 'iot', 'ota', 'mac', 'ip', 'vm', 'faas', 'paas', 'saas', 'iaas', 'iam', 'mec', 'ml', 'ble', 'rfid', 'gps', 'nft', 'dao', 'zk', 'evm', 'wasm', 'etl', 'olap', 'ci', 'cd', 'apm', 'sre', 'iac', 'dma'}
    
    title_words = []
    for w in words:
        if w.lower() in special_acronyms:
            title_words.append(w.upper())
        else:
            title_words.append(w.capitalize())
            
    return ' '.join(title_words)

def generate_en_dict(zh_dict):
    """Generate English dictionary based on the structure of the Chinese one."""
    en_dict = {}
    for k, v in zh_dict.items():
        if isinstance(v, dict):
            en_dict[k] = generate_en_dict(v)
        else:
            en_dict[k] = to_english_title(k)
    return en_dict

import urllib.request
import urllib.parse
import time

def translate_text(text, target_lang="ru", source_lang="en", max_retries=3):
    """Translate text using free Google Translate API with retry mechanism."""
    # Only translate if there's actual text
    if not text or text == "None":
        return text
        
    url = f"https://translate.googleapis.com/translate_a/single?client=gtx&sl={source_lang}&tl={target_lang}&dt=t&q={urllib.parse.quote(text)}"
    
    for attempt in range(max_retries):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
            response = urllib.request.urlopen(req, timeout=5)
            data = json.loads(response.read().decode('utf-8'))
            # Give Google a tiny break to prevent rate limits
            time.sleep(0.1)
            return data[0][0][0]
        except Exception as e:
            if attempt < max_retries - 1:
                print(f"⚠️ Translation failed for '{text}' ({e}). Retrying ({attempt + 1}/{max_retries})...")
                time.sleep(1) # Wait before retrying
            else:
                print(f"❌ Translation failed for '{text}' after {max_retries} attempts: {e}")
                return text

def generate_translated_dict(en_dict, target_lang):
    """Recursively translate the English dictionary into the target language."""
    translated = {}
    for k, v in en_dict.items():
        if isinstance(v, dict):
            translated[k] = generate_translated_dict(v, target_lang)
        else:
            print(f"Translating: {v} -> {target_lang}...")
            translated[k] = translate_text(v, target_lang=target_lang)
    return translated

def main():
    parser = argparse.ArgumentParser(description="Generate i18n JSON files from C++ enum comments.")
    parser.add_argument('--lang', default='all', help="Specify which language to generate (e.g., zh_CN, en_US, ru_RU, ja_JP, all).")
    args = parser.parse_args()

    root = get_project_root()
    include_dir = os.path.join(root, "include", "error_system")
    
    zh_cn = {
        "error_level": {},
        "domain": {},
        "subsystem": {},
        "module": {}
    }

    # 1. Parse error_level
    error_level_file = os.path.join(include_dir, "core", "error_level.h")
    if os.path.exists(error_level_file):
        zh_cn["error_level"] = parse_enum_comments(error_level_file)

    # 2. Parse domain
    domain_file = os.path.join(include_dir, "domain", "system_domain.h")
    if os.path.exists(domain_file):
        zh_cn["domain"] = parse_enum_comments(domain_file)

    # 3. Parse subsystems
    subsystem_dir = os.path.join(include_dir, "subsystem")
    if os.path.exists(subsystem_dir):
        for filename in sorted(os.listdir(subsystem_dir)):
            if filename.endswith("_subsystem.h"):
                domain_name = filename.replace("_subsystem.h", "")
                filepath = os.path.join(subsystem_dir, filename)
                zh_cn["subsystem"][domain_name] = parse_enum_comments(filepath)

    # 4. Parse modules
    module_dir = os.path.join(include_dir, "module")
    if os.path.exists(module_dir):
        for filename in sorted(os.listdir(module_dir)):
            if filename.endswith("_module.h"):
                domain_name = filename.replace("_module.h", "")
                filepath = os.path.join(module_dir, filename)
                zh_cn["module"][domain_name] = parse_enum_comments(filepath)

    # Output files
    lang_dir = os.path.join(include_dir, "i18n", "languages")
    os.makedirs(lang_dir, exist_ok=True)
    
    generated_files = []
    
    # Always generate en_US first in memory because it acts as the translation source
    en_us = generate_en_dict(zh_cn)

    # Generate standard languages
    if args.lang in ['zh_CN', 'all']:
        zh_file = os.path.join(lang_dir, "zh_CN.json")
        with open(zh_file, 'w', encoding='utf-8') as f:
            json.dump(zh_cn, f, ensure_ascii=False, indent=4)
        generated_files.append(zh_file)
        
    if args.lang in ['en_US', 'all']:
        en_file = os.path.join(lang_dir, "en_US.json")
        with open(en_file, 'w', encoding='utf-8') as f:
            json.dump(en_us, f, ensure_ascii=False, indent=4)
        generated_files.append(en_file)

    # Generate arbitrary translation if asked (e.g. ru_RU, ja_JP, es_ES)
    if args.lang not in ['zh_CN', 'en_US', 'all']:
        print(f"🌍 Starting translation for '{args.lang}' via Google Translate API...")
        # Extract the language prefix (e.g., 'ru' from 'ru_RU')
        lang_prefix = args.lang.split('_')[0].lower()
        translated_dict = generate_translated_dict(en_us, target_lang=lang_prefix)
        
        target_file = os.path.join(lang_dir, f"{args.lang}.json")
        with open(target_file, 'w', encoding='utf-8') as f:
            json.dump(translated_dict, f, ensure_ascii=False, indent=4)
        generated_files.append(target_file)

    print(f"✅ Successfully generated {len(generated_files)} i18n JSON files!")
    for f in generated_files:
        print(f"  - {f}")

if __name__ == "__main__":
    main()
