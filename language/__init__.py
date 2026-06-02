import json
from pathlib import Path

from config import cfg

_language_cache: dict[str, dict] = {}

def _load_language(language_code: str) -> dict:
    if language_code not in _language_cache:
        with open(f"./assets/language/{language_code}.json", "r", encoding="utf-8") as f:
            _language_cache[language_code] = json.load(f)
    return _language_cache[language_code]


def _get_nested(data: dict, key: str):
    value = data
    for part in key.split("."):
        if not isinstance(value, dict) or part not in value:
            return None
        value = value[part]
    return value


def get_languages() -> list[dict[str, str]]:
    languages = []
    for path in sorted(Path("./assets/language").glob("*.json")):
        try:
            data = _load_language(path.stem)
        except Exception:
            continue
        meta = data.get("meta", {})
        code = str(meta.get("code") or path.stem)
        languages.append(
            {
                "code": code,
                "name": str(meta.get("name") or code),
                "native_name": str(meta.get("native_name") or meta.get("name") or code),
            }
        )
    return languages


def tr(key: str, language_code: str | None = None) -> str:
    if language_code is None:
        language_code = cfg.config.language

    try:
        value = _get_nested(_load_language(language_code), key)
    except Exception:
        value = None

    if value is not None:
        return str(value)
    return key
