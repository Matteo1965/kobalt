                                       {"hozzátéve", "hozzátesz"},
                                       {"feleségemmel", "feleség"},
                                       {"lehessen", "lehet"},
                                       {"mosolyú", "mosoly"},
                                       {"rájöttem", "rájön"},
                                       {"népszerűségük", "népszerűség"},
                                       {"zsákmánnyal", "zsákmány"},
                                       {"vezethető", "vezet"},
                                       {"foglalkoznak", "foglalkozik"},
                                       {"életrajzi", "életrajz"},
                                       {"életén", "élet"},
                                       {"fellelhető", "fellel"},
                                       {"bírósági", "bíróság"},
                                       {"hadihajóinak", "hadihajó"},
                                       {"kutatásom", "kutatás"},
                                       {"támpontunk", "támpont"},
                                       {"mentoruk", "mentor"},
                                       {"gyakorlatilag", "gyakorlat"},
                                       {"karrierjük", "karrier"},
                                       {"mindegyikük", "mindegyik"},
                                       {"szállítmányozási", "szállítmányozás"}};
    for (const auto& p : SPECIAL)
      if (w == p.from) add(p.to);

    if (w.size() > 2 && w.compare(w.size() - 2, 2, "-e") == 0) add(w.substr(0, w.size() - 2));
    if (ends("nek")) add(w.substr(0, w.size() - 3) + "ik");

    for (const char* s : {"nia", "nie"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s : {"va", "ve"}) {
      std::string st;
      if (strip(w, s, st)) {
        add(st + "ik");
        add(st);
      }
    }

    if (ends("ák")) add(w.substr(0, w.size() - strlen("ák")) + "a");
    if (ends("ék")) add(w.substr(0, w.size() - strlen("ék")) + "e");

    for (const char* s : {"ak", "ek", "ok", "ök"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s :
         {"jait", "jeit", "ait", "eit", "ját", "jét", "át", "ét", "jai", "jei", "ai", "ei", "ja", "je", "a", "e"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s :
         {"eteket", "atokat", "otokat", "ötöket", "talanok", "telenek", "jetek", "jatok", "jen", "jön"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }

    if (ends("ele")) add(w.substr(0, w.size() - 3) + "él");
    if (ends("ormok")) add(w.substr(0, w.size() - strlen("ormok")) + "orom");
    if (ends("epén")) add(w.substr(0, w.size() - strlen("epén")) + "ép");
    if (ends("leit")) add(w.substr(0, w.size() - strlen("leit")) + "l");
    if (w == "beleessen") add("beleesik");

    for (const char* s : {"tan", "ten"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s : {"ását", "ését"}) {
      std::string st;
      if (!strip(w, s, st)) continue;
      add(st + "ás");
      add(st + "és");
      add(st);
      if (!st.empty() && st.back() == 'g') {
        auto b = st.substr(0, st.size() - 1);
        add(b + "og");
        add(b + "eg");
        add(b + "ög");
      }
    }

    for (const char* s : {"kal", "kel"}) {
      std::string pl;
      if (!strip(w, s, pl)) continue;
      add(pl);
      for (const char* ps : PLURAL) {
        std::string st;
        if (!strip(pl, ps, st)) continue;
        add(st);
        if (st.size() >= strlen("öv") && st.compare(st.size() - strlen("öv"), strlen("öv"), "öv") == 0)
          add(st.substr(0, st.size() - strlen("öv")) + "ő");
      }
    }

    for (const char* s : {"séggel", "sággal"}) {
      std::string st;
      if (strip(w, s, st)) {
        add(st + "s");
        add(st);
      }
    }
    for (const char* s : {"úság", "űség"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    if (ends("sz")) add(w.substr(0, w.size() - strlen("sz")));

    for (const char* s : {"bbá", "bbé"}) {
      std::string st;
      if (!strip(w, s, st)) continue;
      if (st.size() >= strlen("á") && st.compare(st.size() - strlen("á"), strlen("á"), "á") == 0)
        add(st.substr(0, st.size() - strlen("á")) + "a");
      if (st.size() >= strlen("é") && st.compare(st.size() - strlen("é"), strlen("é"), "é") == 0)
        add(st.substr(0, st.size() - strlen("é")) + "e");
      add(st);
    }

    for (const char* s : {"ősen", "ósan", "özött", "ozott", "ezett"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    if (ends("ikat")) add(w.substr(0, w.size() - strlen("ikat")));

    if (w.rfind("leg", 0) == 0 && (ends("abb") || ends("ebb") || ends("obb"))) add(w.substr(3, w.size() - 6));
    for (const char* s : {"ésén", "ásán"})
      if (ends(s)) add(w.substr(0, w.size() - 2));
    if (ends("essé")) add(w.substr(0, w.size() - 2));
    if (ends("ságban") || ends("ségben")) add(w.substr(0, w.size() - 3));

    for (const char* s : {"olják", "eljék", "öljék"}) {
      std::string st;
      if (strip(w, s, st)) add(st + std::string(s).substr(0, 2));
    }
    for (const char* s : {"sebbek", "sabbak"}) {
      std::string st;
      if (strip(w, s, st)) add(st + "s");
    }
    if (ends("nább")) add(w.substr(0, w.size() - 4) + "na");
    if (ends("nébb")) add(w.substr(0, w.size() - 4) + "ne");
    if (ends("kori")) add(w.substr(0, w.size() - 1));
    if (ends("állá")) add(w.substr(0, w.size() - 1));

    for (const char* s : {"ással", "éssel"})
      if (ends(s)) add(w.substr(0, w.size() - 4));
    for (const char* s : {"hattak", "hettek"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    if (ends("tság") || ends("tség")) add(w.substr(0, w.size() - 4));
    if (ends("n")) add(w.substr(0, w.size() - 1));

    for (const char* s : {"ozni", "ezni"}) {
      if (ends(s)) add(w.substr(0, w.size() - 4) + std::string(s).substr(0, 2) + "ik");
    }
    if (ends("ni")) add(w.substr(0, w.size() - 2));
    for (const char* s : {"an", "en", "abb", "ebb", "obb"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }
    for (const char* s : {"ával", "ével", "val", "vel"}) {
      std::string st;
      if (strip(w, s, st)) add(st);
    }

    if (w.size() >= 4 && (w.compare(w.size() - 2, 2, "al") == 0 || w.compare(w.size() - 2, 2, "el") == 0) &&
        w[w.size() - 3] == w[w.size() - 4])
      add(w.substr(0, w.size() - 3));

