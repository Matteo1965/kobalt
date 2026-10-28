      }
    }
  };

  std::vector<std::string> queue;
  generateOne(word, queue);
  std::vector<std::string> seen;
  for (int round = 0; round < 3 && !queue.empty() && out.size() < MAX_STEM_VARIANTS; ++round) {
    std::vector<std::string> next;
    for (const auto& cand : queue) {
      if (std::find(seen.begin(), seen.end(), cand) != seen.end()) continue;
      seen.push_back(cand);
      size_t candCodepoints = 0;
      for (const unsigned char ch : cand) if ((ch & 0xC0) != 0x80) ++candCodepoints;
      if (candCodepoints >= 3 && out.size() < MAX_STEM_VARIANTS) addUnique(out, cand);
      std::vector<std::string> more;
      generateOne(cand, more);
      for (const auto& m : more) addUnique(next, m);
    }
    queue.swap(next);
  }

  // Compound fallback is always last, and longest suffixes are emitted first.
  std::vector<std::string> compound;
  std::vector<std::string> sources;
  sources.push_back(word);
  sources.insert(sources.end(), seen.begin(), seen.end());
  for (const auto& source : sources) {
    for (size_t i = 1; i < source.size(); ++i) {
      if ((static_cast<unsigned char>(source[i]) & 0xC0) == 0x80) continue;
      const std::string tail = source.substr(i);
      size_t cps = 0;
      for (unsigned char ch : tail)
        if ((ch & 0xC0) != 0x80) ++cps;
      if (cps >= 6) addUnique(compound, tail);
    }
  }
  std::stable_sort(compound.begin(), compound.end(),
