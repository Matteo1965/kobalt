      add(base);
      for (const char* ps : PLURAL) {
        std::string st;
        if (strip(base, ps, st)) add(st);
      }
      if (base.size() >= strlen("á") && base.compare(base.size() - strlen("á"), strlen("á"), "á") == 0) {
        add(base.substr(0, base.size() - strlen("á")) + "a");
        add(base.substr(0, base.size() - strlen("á")));
      }
      if (base.size() >= strlen("é") && base.compare(base.size() - strlen("é"), strlen("é"), "é") == 0) {
        add(base.substr(0, base.size() - strlen("é")) + "e");
        add(base.substr(0, base.size() - strlen("é")));
      }
    }

    for (const char* s : {"at", "et", "ot", "öt", "t"}) {
      std::string base;
      if (!strip(w, s, base)) continue;
      add(base);
      if (!base.empty() && base.back() == 'u') add(base.substr(0, base.size() - 1) + "ú");
      if (!base.empty() && base.back() == 'e') add(base.substr(0, base.size() - 1) + "é");
      if (!base.empty() && base.back() == 'a') add(base.substr(0, base.size() - 1) + "á");
    }
    for (const char* ps : PLURAL) {
      std::string st;
      if (strip(w, ps, st)) add(st);
    }
    for (const char* s : PAST) {
      std::string st;
      if (strip(w, s, st)) {
        add(st + "ik");
        add(st);
      }
    }
    for (const char* s : {"ás", "és"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s : {"ó", "ő"}) {
      std::string st;
      if (strip(w, s, st)) {
        add(st + "ik");
        add(st);
