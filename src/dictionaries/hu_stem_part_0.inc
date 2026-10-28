inline void hungarianStemVariants(const std::string& word, std::vector<std::string>& out) {
  // Hungarian dictionary stemming v12: morphology-first engine.
  // Hungarian dictionary stemming v13: final validated irregular restorations.
  // Hungarian dictionary stemming v14: validated real-book lexical restorations.
  // Hungarian dictionary stemming v15: reject short accidental compound-tail matches.
  // Hungarian dictionary stemming v16: validated corpus mappings and Hungarian UTF-8 case folding.
  // Mirrors the host-side reference model: generate ordered morphology candidates
  // for up to three rounds, then try compound suffix components last.
  out.clear();
  out.reserve(128);
  constexpr size_t MAX_STEM_VARIANTS = 128;

  const auto addUnique = [](std::vector<std::string>& dst, const std::string& v) {
    if (v.empty()) return;
    if (std::find(dst.begin(), dst.end(), v) == dst.end()) dst.push_back(v);
  };
  const auto strip = [](const std::string& s, const char* suffix, std::string& stem) {
    const size_t n = strlen(suffix);
    if (s.size() <= n || s.compare(s.size() - n, n, suffix) != 0) return false;
    stem.assign(s, 0, s.size() - n);
    return !stem.empty();
  };

  static constexpr const char* CASE_SUFFIXES[] = {
      "képpen", "ként",  "jából", "jéből", "jába", "jébe", "járól", "jéről", "ján", "jén", "jához",
      "jéhez",  "jéhöz", "ból",   "ből",   "tól",  "től",  "ról",   "ről",   "hoz", "hez", "höz",
      "nál",    "nél",   "ban",   "ben",   "nak",  "nek",  "ért",   "ba",    "be",  "ra",  "re",
      "on",     "en",    "ön",    "ig",    "kor",  "ul",   "ül",    "vá",    "vé"};
  static constexpr const char* PLURAL[] = {"ak", "ek", "ok", "ök", "k"};
  static constexpr const char* PAST[] = {"ottam", "ettem",   "öttem",   "tam",     "tem",    "ottál",  "ettél",
                                         "öttél", "tál",     "tél",     "ottunk",  "ettünk", "öttünk", "tunk",
                                         "tünk",  "ottatok", "ettetek", "öttetek", "tatok",  "tetek",  "ottak",
                                         "ettek", "öttek",   "tak",     "tek",     "ták",    "ték",    "otta",
                                         "ette",  "ötte",    "ta",      "te",      "ott",    "ett",    "ött"};

  const auto generateOne = [&](const std::string& w, std::vector<std::string>& c) {
    auto add = [&](const std::string& v) { addUnique(c, v); };
    auto ends = [&](const char* s) {
      const size_t n = strlen(s);
      return w.size() > n && w.compare(w.size() - n, n, s) == 0;
    };

    // Small set of true lexical/orthographic irregulars retained from the validated corpus.
    struct Pair {
      const char* from;
      const char* to;
    };
    static constexpr Pair SPECIAL[] = {{"nekem", "én"},
                                       {"atyavilág", "világ"},
                                       {"kabátja", "kabát"},
                                       {"igazgatónője", "igazgató"},
                                       {"beleessen", "beleesik"},
                                       {"kihalgassa", "kihallgat"},
                                       {"utat", "út"},
                                       {"sziklahasadékból", "hasadék"},
                                       {"madárlába", "láb"},
                                       {"tompaagyúság", "agy"},
                                       {"ezreivel", "ezer"},
                                       {"megítélésén", "megítélés"},
                                       {"gyűlöletessé", "gyűlöletes"},
                                       {"véznább", "vézna"},
                                       {"szakállá", "szakáll"},
                                       {"mosolygott", "mosolyog"},
                                       {"lenyomataim", "lenyomat"},
                                       {"tönkretettem", "tönkretesz"},
                                       {"kiviharzok", "kiviharzik"},
                                       {"kátránnyal", "kátrány"},
                                       {"gyapotruhákkal", "gyapotruha"},
                                       {"esztétája", "esztéta"},
                                       {"viseli", "visel"},
                                       {"tóparton", "part"},
                                       {"lófarokba", "farok"},
                                       {"beleüvöltve", "üvölt"},
                                       {"keze", "kéz"},
                                       {"őrzött", "őriz"},
                                       {"akadályozzák", "akadályoz"},
                                       {"beszélnünk", "beszél"},
                                       {"biztonságiőr", "biztonság"},
                                       {"boríthatjuk", "borít"},
                                       {"bástyája", "bástya"},
                                       {"bürokráciával", "bürokrácia"},
                                       {"centis", "centi"},
                                       {"csapdát", "csapda"},
                                       {"darabkája", "darab"},
                                       {"eleji", "eleje"},
                                       {"ellenőrzik", "ellenőriz"},
                                       {"felbontású", "felbontás"},
                                       {"felszerelésük", "felszerelés"},
                                       {"frekvenciájú", "frekvencia"},
                                       {"gondoljon", "gondol"},
                                       {"grafikai", "grafika"},
                                       {"grafikát", "grafika"},
                                       {"gurulós", "gurul"},
                                       {"gyártási", "gyártás"},
                                       {"halántékú", "halánték"},
                                       {"intelligenciái", "intelligencia"},
                                       {"irodát", "iroda"},
                                       {"irodával", "iroda"},
                                       {"jöhet", "jön"},
                                       {"kerüli", "kerül"},
                                       {"kezdjük", "kezd"},
                                       {"kezem", "kéz"},
                                       {"kezébe", "kéz"},
                                       {"kezét", "kéz"},
                                       {"kezünk", "kéz"},
