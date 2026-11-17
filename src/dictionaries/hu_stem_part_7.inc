                   [](const std::string& a, const std::string& b) { return a.size() > b.size(); });
  for (const auto& c : compound)
    if (out.size() < MAX_STEM_VARIANTS) addUnique(out, c);
}

