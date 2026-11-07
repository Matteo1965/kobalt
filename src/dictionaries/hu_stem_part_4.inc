    for (const char* s : {"ásával", "ésével"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s : {"otta", "ette", "ötte", "olta", "elte"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }

    for (const char* s : {"tokból", "tekből", "tökből"}) {
      std::string st;
      if (!strip(w, s, st)) continue;
      add(st);
      if (st.size() >= strlen("szá") && st.compare(st.size() - strlen("szá"), strlen("szá"), "szá") == 0)
        add(st.substr(0, st.size() - strlen("szá")) + "száj");
    }

    for (const char* s : {"gatják", "getik", "gatja", "geti"}) {
      std::string st;
      if (strip(w, s, st)) add(st + std::string(s).substr(0, 3));
    }
    for (const char* s : {"hatunk", "hetünk", "hattunk", "hettünk"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }

    if (ends("fiát")) add(w.substr(0, w.size() - strlen("fiát")) + "fiú");
    if (ends("-beli")) add(w.substr(0, w.size() - strlen("-beli")));
    if (ends("ebben")) {
      auto st = w.substr(0, w.size() - strlen("ebben"));
      add(st + "ű");
      add(st);
    }

    for (const char* s : {"zzanak", "zzenek"}) {
      std::string st;
      if (strip(w, s, st)) {
        add(st + "zik");
        add(st + "ik");
        add(st);
      }
    }

    if (ends("zzák")) {
      add(w.substr(0, w.size() - strlen("zák")));
    }

    for (const char* s : CASE_SUFFIXES) {
      std::string base;
      if (!strip(w, s, base)) continue;
