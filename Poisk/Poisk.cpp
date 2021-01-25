#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();
	return result;
}

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;
	for (const char c : text) {
		if (c == ' ') {
			words.push_back(word);
			word = "";
		}
		else {
			word += c;
		}
	}
	words.push_back(word);

	return words;
}

struct Document {
	int id;
	float relevance;
};

struct Query {
	vector<string> plus_words;
	vector<string> minus_words;
};

class SearchServer {
public:
	void SetStopWords(const string& text) {
		for (const string& word : SplitIntoWords(text)) {
			stop_words_.insert(word);
		}
	}

	void AddDocument(int document_id, const string& document) {
		for (const string& word : SplitIntoWordsNoStop(document)) {
			word_to_documents_[word].insert(document_id);
			document_words_[document_id].push_back(word);
		}
	}

	vector<Document> FindTopDocuments(const string& query) const {
		auto matched_documents = FindAllDocuments(query);

		sort(
			matched_documents.begin(),
			matched_documents.end(),
			[](const Document& lhs, const Document& rhs) {
				return lhs.relevance > rhs.relevance;
			}
		);
		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
		return matched_documents;
	}

private:
	map<string, set<int>> word_to_documents_;
	set<string> stop_words_;
	map<int, vector<string>> document_words_;

	vector<string> SplitIntoWordsNoStop(const string& text) const {
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			if (stop_words_.count(word) == 0) {
				words.push_back(word);
			}
		}
		return words;
	}

	Query ParseQuery(const string& text) const {
		Query words;
		for (const string& word : SplitIntoWords(text)) {
			if (stop_words_.count(word) == 0) {
				if (word[0] != '-') {
					words.plus_words.push_back(word);
				}
				else {
					words.minus_words.push_back(word.substr(1));
				}
			}
		}
		return words;
	}

	vector<Document> FindAllDocuments(const string& query) const {
		const Query query_words = ParseQuery(query);
		map<int, float> document_to_relevance;
		map<string, float> word_IDF;
		for (const auto& word : query_words.plus_words) {
			if (word_to_documents_.count(word) == 0) {
				continue;
			}
			word_IDF[word] = log(document_words_.size() * 1.0 / word_to_documents_.at(word).size());
		}

		for (int i = 0; i < document_words_.size(); i++) {
			for (const auto& query_word : query_words.plus_words) {
				float count = 0;
				for (const auto& word : document_words_.at(i)) {
					if (query_word == word) {
						count++;
					}
				}
				document_to_relevance[i] += count / document_words_.at(i).size() * word_IDF[query_word];
			}
		}

		for (const auto& word : query_words.minus_words) {
			if (word_to_documents_.count(word) == 0) {
				continue;
			}
			for (const int document_id : word_to_documents_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}

		vector<Document> matched_documents;
		for (auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back({ document_id, relevance });
		}

		return matched_documents;
	}
};

SearchServer CreateSearchServer() {
	SearchServer search_server;
	search_server.SetStopWords(ReadLine());

	const int document_count = ReadLineWithNumber();
	for (int document_id = 0; document_id < document_count; ++document_id) {
		search_server.AddDocument(document_id, ReadLine());
	}

	return search_server;
}


int main() {
	const SearchServer search_server = CreateSearchServer();

	const string query = ReadLine();
	for (auto [document_id, relevance] : search_server.FindTopDocuments(query)) {
		cout << "{ document_id = " << document_id << ", relevance = " << relevance << " }" << endl;
	}
}