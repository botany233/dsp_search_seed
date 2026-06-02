import json
from pathlib import Path

FALLBACK_LANGUAGE = "zh_CN"
DEFAULT_DOMAIN_CATEGORIES = [
    "sort_types",
    "custom_sort_types",
    "resource_rates",
    "veins",
    "gas_veins",
    "liquids",
    "planet_types",
    "star_types",
    "singularity",
    "dsp_levels",
    "dsp_planets",
    "condition_names",
]

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
        from config import cfg

        language_code = cfg.config.language

    for code in (language_code, FALLBACK_LANGUAGE):
        try:
            value = _get_nested(_load_language(code), key)
        except Exception:
            value = None
        if value is not None:
            return str(value)
    return key

def tr_domain(category: str, value: str, language_code: str | None = None) -> str:
    if language_code is None:
        from config import cfg

        language_code = cfg.config.language

    for code in (language_code, FALLBACK_LANGUAGE):
        try:
            domain = _get_nested(_load_language(code), f"domain.{category}")
        except Exception:
            domain = None
        if isinstance(domain, dict) and value in domain:
            return str(domain[value])
    return str(value)

def tr_any_domain(value: str, categories: list[str] | tuple[str, ...] | None = None) -> str:
    for category in categories or DEFAULT_DOMAIN_CATEGORIES:
        translated = tr_domain(category, value)
        if translated != value:
            return translated
    return str(value)
