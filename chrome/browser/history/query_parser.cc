// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/history/query_parser.h"

#include <algorithm>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/i18n/break_iterator.h"
#include "base/i18n/case_conversion.h"
#include "base/logging.h"
#include "base/stl_util-inl.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

// Returns true if |mp1.first| is less than |mp2.first|. This is used to
// sort match positions.
int CompareMatchPosition(const Snippet::MatchPosition& mp1,
                         const Snippet::MatchPosition& mp2) {
  return mp1.first < mp2.first;
}

// Returns true if |mp2| intersects |mp1|. This is intended for use by
// CoalesceMatchesFrom and isn't meant as a general intersectpion comparison
// function.
bool SnippetIntersects(const Snippet::MatchPosition& mp1,
                       const Snippet::MatchPosition& mp2) {
  return mp2.first >= mp1.first && mp2.first <= mp1.second;
}

// Coalesces match positions in |matches| after index that intersect the match
// position at |index|.
void CoalesceMatchesFrom(size_t index, Snippet::MatchPositions* matches) {
  Snippet::MatchPosition& mp = (*matches)[index];
  for (Snippet::MatchPositions::iterator i = matches->begin() + index + 1;
       i != matches->end(); ) {
    if (SnippetIntersects(mp, *i)) {
      mp.second = i->second;
      i = matches->erase(i);
    } else {
      return;
    }
  }
}

// Sorts the match positions in |matches| by their first index, then coalesces
// any match positions that intersect each other.
void CoalseAndSortMatchPositions(Snippet::MatchPositions* matches) {
  std::sort(matches->begin(), matches->end(), &CompareMatchPosition);
  // WARNING: we don't use iterator here as CoalesceMatchesFrom may remove
  // from matches.
  for (size_t i = 0; i < matches->size(); ++i)
    CoalesceMatchesFrom(i, matches);
}

// Returns true if the character is considered a quote.
bool IsQueryQuote(wchar_t ch) {
  return ch == '"' ||
         ch == 0xab ||    // left pointing double angle bracket
         ch == 0xbb ||    // right pointing double angle bracket
         ch == 0x201c ||  // left double quotation mark
         ch == 0x201d ||  // right double quotation mark
         ch == 0x201e;    // double low-9 quotation mark
}

}  // namespace

// Inheritance structure:
// Queries are represented as trees of QueryNodes.
// QueryNodes are either a collection of subnodes (a QueryNodeList)
// or a single word (a QueryNodeWord).

// A QueryNodeWord is a single word in the query.
class QueryNodeWord : public QueryNode {
 public:
  explicit QueryNodeWord(const string16& word);
  virtual ~QueryNodeWord();

  const string16& word() const { return word_; }

  void set_literal(bool literal) { literal_ = literal; }

  // QueryNode:
  virtual int AppendToSQLiteQuery(string16* query) const OVERRIDE;
  virtual bool IsWord() const OVERRIDE;
  virtual bool Matches(const string16& word, bool exact) const OVERRIDE;
  virtual bool HasMatchIn(
      const std::vector<QueryWord>& words,
      Snippet::MatchPositions* match_positions) const OVERRIDE;
  virtual void AppendWords(std::vector<string16>* words) const OVERRIDE;

 private:
  string16 word_;
  bool literal_;

  DISALLOW_COPY_AND_ASSIGN(QueryNodeWord);
};

QueryNodeWord::QueryNodeWord(const string16& word)
    : word_(word),
      literal_(false) {}

QueryNodeWord::~QueryNodeWord() {}

int QueryNodeWord::AppendToSQLiteQuery(string16* query) const {
  query->append(word_);

  // Use prefix search if we're not literal and long enough.
  if (!literal_ && QueryParser::IsWordLongEnoughForPrefixSearch(word_))
    *query += L'*';
  return 1;
}

bool QueryNodeWord::IsWord() const {
  return true;
}

bool QueryNodeWord::Matches(const string16& word, bool exact) const {
  if (exact || !QueryParser::IsWordLongEnoughForPrefixSearch(word_))
    return word == word_;
  return word.size() >= word_.size() &&
         (word_.compare(0, word_.size(), word, 0, word_.size()) == 0);
}

bool QueryNodeWord::HasMatchIn(const std::vector<QueryWord>& words,
                               Snippet::MatchPositions* match_positions) const {
  for (size_t i = 0; i < words.size(); ++i) {
    if (Matches(words[i].word, false)) {
      size_t match_start = words[i].position;
      match_positions->push_back(
          Snippet::MatchPosition(match_start,
                                 match_start + static_cast<int>(word_.size())));
      return true;
    }
  }
  return false;
}

void QueryNodeWord::AppendWords(std::vector<string16>* words) const {
  words->push_back(word_);
}

// A QueryNodeList has a collection of QueryNodes which are deleted in the end.
class QueryNodeList : public QueryNode {
 public:
  typedef std::vector<QueryNode*> QueryNodeVector;

  QueryNodeList();
  virtual ~QueryNodeList();

  QueryNodeVector* children() { return &children_; }

  void AddChild(QueryNode* node);

  // Remove empty subnodes left over from other parsing.
  void RemoveEmptySubnodes();

  // QueryNode:
  virtual int AppendToSQLiteQuery(string16* query) const OVERRIDE;
  virtual bool IsWord() const OVERRIDE;
  virtual bool Matches(const string16& word, bool exact) const OVERRIDE;
  virtual bool HasMatchIn(
      const std::vector<QueryWord>& words,
      Snippet::MatchPositions* match_positions) const OVERRIDE;
  virtual void AppendWords(std::vector<string16>* words) const OVERRIDE;

 protected:
  int AppendChildrenToString(string16* query) const;

  QueryNodeVector children_;

 private:
  DISALLOW_COPY_AND_ASSIGN(QueryNodeList);
};

QueryNodeList::QueryNodeList() {}

QueryNodeList::~QueryNodeList() {
  STLDeleteElements(&children_);
}

void QueryNodeList::AddChild(QueryNode* node) {
  children_.push_back(node);
}

void QueryNodeList::RemoveEmptySubnodes() {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (children_[i]->IsWord())
      continue;

    QueryNodeList* list_node = static_cast<QueryNodeList*>(children_[i]);
    list_node->RemoveEmptySubnodes();
    if (list_node->children()->empty()) {
      children_.erase(children_.begin() + i);
      --i;
      delete list_node;
    }
  }
}

int QueryNodeList::AppendToSQLiteQuery(string16* query) const {
  return AppendChildrenToString(query);
}

bool QueryNodeList::IsWord() const {
  return false;
}

bool QueryNodeList::Matches(const string16& word, bool exact) const {
  NOTREACHED();
  return false;
}

bool QueryNodeList::HasMatchIn(const std::vector<QueryWord>& words,
                               Snippet::MatchPositions* match_positions) const {
  NOTREACHED();
  return false;
}

void QueryNodeList::AppendWords(std::vector<string16>* words) const {
  for (size_t i = 0; i < children_.size(); ++i)
    children_[i]->AppendWords(words);
}

int QueryNodeList::AppendChildrenToString(string16* query) const {
  int num_words = 0;
  for (QueryNodeVector::const_iterator node = children_.begin();
       node != children_.end(); ++node) {
    if (node != children_.begin())
      query->push_back(L' ');
    num_words += (*node)->AppendToSQLiteQuery(query);
  }
  return num_words;
}

// A QueryNodePhrase is a phrase query ("quoted").
class QueryNodePhrase : public QueryNodeList {
 public:
  QueryNodePhrase();
  virtual ~QueryNodePhrase();

  // QueryNodeList:
  virtual int AppendToSQLiteQuery(string16* query) const OVERRIDE;
  virtual bool HasMatchIn(
      const std::vector<QueryWord>& words,
      Snippet::MatchPositions* match_positions) const OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(QueryNodePhrase);
};

QueryNodePhrase::QueryNodePhrase() {}

QueryNodePhrase::~QueryNodePhrase() {}

int QueryNodePhrase::AppendToSQLiteQuery(string16* query) const {
  query->push_back(L'"');
  int num_words = AppendChildrenToString(query);
  query->push_back(L'"');
  return num_words;
}

bool QueryNodePhrase::HasMatchIn(
    const std::vector<QueryWord>& words,
    Snippet::MatchPositions* match_positions) const {
  if (words.size() < children_.size())
    return false;

  for (size_t i = 0, max = words.size() - children_.size() + 1; i < max; ++i) {
    bool matched_all = true;
    for (size_t j = 0; j < children_.size(); ++j) {
      if (!children_[j]->Matches(words[i + j].word, true)) {
        matched_all = false;
        break;
      }
    }
    if (matched_all) {
      const QueryWord& last_word = words[i + children_.size() - 1];
      match_positions->push_back(
          Snippet::MatchPosition(words[i].position,
                                 last_word.position + last_word.word.length()));
      return true;
    }
  }
  return false;
}

QueryParser::QueryParser() {}

// static
bool QueryParser::IsWordLongEnoughForPrefixSearch(const string16& word) {
  DCHECK(!word.empty());
  size_t minimum_length = 3;
  // We intentionally exclude Hangul Jamos (both Conjoining and compatibility)
  // because they 'behave like' Latin letters. Moreover, we should
  // normalize the former before reaching here.
  if (0xAC00 <= word[0] && word[0] <= 0xD7A3)
    minimum_length = 2;
  return word.size() >= minimum_length;
}

int QueryParser::ParseQuery(const string16& query, string16* sqlite_query) {
  QueryNodeList root;
  if (!ParseQueryImpl(query, &root))
    return 0;
  return root.AppendToSQLiteQuery(sqlite_query);
}

void QueryParser::ParseQuery(const string16& query,
                             std::vector<QueryNode*>* nodes) {
  QueryNodeList root;
  if (ParseQueryImpl(base::i18n::ToLower(query), &root))
    nodes->swap(*root.children());
}


void QueryParser::ExtractQueryWords(const string16& query,
                                    std::vector<string16>* words) {
  QueryNodeList root;
  if (!ParseQueryImpl(query, &root))
    return;
  root.AppendWords(words);
}

bool QueryParser::DoesQueryMatch(const string16& text,
                                 const std::vector<QueryNode*>& query_nodes,
                                 Snippet::MatchPositions* match_positions) {
  if (query_nodes.empty())
    return false;

  std::vector<QueryWord> query_words;
  string16 lower_text = base::i18n::ToLower(text);
  ExtractQueryWords(lower_text, &query_words);

  if (query_words.empty())
    return false;

  Snippet::MatchPositions matches;
  for (size_t i = 0; i < query_nodes.size(); ++i) {
    if (!query_nodes[i]->HasMatchIn(query_words, &matches))
      return false;
  }
  if (lower_text.length() != text.length()) {
    // The lower case string differs from the original string. The matches are
    // meaningless.
    // TODO(sky): we need a better way to align the positions so that we don't
    // completely punt here.
    match_positions->clear();
  } else {
    CoalseAndSortMatchPositions(&matches);
    match_positions->swap(matches);
  }
  return true;
}

bool QueryParser::ParseQueryImpl(const string16& query, QueryNodeList* root) {
  base::BreakIterator iter(&query, base::BreakIterator::BREAK_WORD);
  // TODO(evanm): support a locale here
  if (!iter.Init())
    return false;

  // To handle nesting, we maintain a stack of QueryNodeLists.
  // The last element (back) of the stack contains the current, deepest node.
  std::vector<QueryNodeList*> query_stack;
  query_stack.push_back(root);

  bool in_quotes = false;  // whether we're currently in a quoted phrase
  while (iter.Advance()) {
    // Just found a span between 'prev' (inclusive) and 'pos' (exclusive). It
    // is not necessarily a word, but could also be a sequence of punctuation
    // or whitespace.
    if (iter.IsWord()) {
      string16 word = iter.GetString();

      QueryNodeWord* word_node = new QueryNodeWord(word);
      if (in_quotes)
        word_node->set_literal(true);
      query_stack.back()->AddChild(word_node);
    } else {  // Punctuation.
      if (IsQueryQuote(query[iter.prev()])) {
        if (!in_quotes) {
          QueryNodeList* quotes_node = new QueryNodePhrase;
          query_stack.back()->AddChild(quotes_node);
          query_stack.push_back(quotes_node);
          in_quotes = true;
        } else {
          query_stack.pop_back();  // Stop adding to the quoted phrase.
          in_quotes = false;
        }
      }
    }
  }

  root->RemoveEmptySubnodes();
  return true;
}

void QueryParser::ExtractQueryWords(const string16& text,
                                    std::vector<QueryWord>* words) {
  base::BreakIterator iter(&text, base::BreakIterator::BREAK_WORD);
  // TODO(evanm): support a locale here
  if (!iter.Init())
    return;

  while (iter.Advance()) {
    // Just found a span between 'prev' (inclusive) and 'pos' (exclusive). It
    // is not necessarily a word, but could also be a sequence of punctuation
    // or whitespace.
    if (iter.IsWord()) {
      string16 word = iter.GetString();
      if (!word.empty()) {
        words->push_back(QueryWord());
        words->back().word = word;
        words->back().position = iter.prev();
      }
    }
  }
}
