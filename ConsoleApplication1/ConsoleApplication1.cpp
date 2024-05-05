#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <cassert>
#include <fstream>
#include <windows.h> 

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void showIntroText() {
	cout << "---------Welcome to our program on Web Crawling CLI!--------- \nIn this cli, you will explore web crawling through the cli. \nGet ready to unlock the potential of web crawling and harness the power of the command line for your data extraction needs.";
}

void setPassword(string& password) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;

	GetConsoleMode(hStdin, &mode);

	SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

	cout << "\nEnter password: ";
	getline(cin, password);

	SetConsoleMode(hStdin, mode);

	cout << "\n"; 
}

struct User {
	string name;
	string password;
	string content;
	string url;

	User(const string& name, const string& password, const string& content, const string& url)
		: name(name), password(password), content(content), url(url) {}
};

class HTMLElement {
public:
	string tagName;
	vector<HTMLElement*> children;
	HTMLElement* parentElement;
	string textContent;
};

enum State {
	STATE_INIT, STATE_START_TAG, STATE_READING_TAG, STATE_READING_ATTRIBUTES, STATE_END_TAG, STATE_BEGIN_CLOSING_TAG
};

bool isWhitespace(char c) {
	return c == ' ' || c == '\n' || c == '\t';
}

HTMLElement* HTMLParser(const string& body) {
	HTMLElement* root = new HTMLElement();
	HTMLElement* currentElement = root;
	vector<HTMLElement*> elementStack;
	State state = STATE_INIT;
	string tagName = "";

	for (char c : body) {
		switch (state) {
		case STATE_INIT:
			if (c == '<') {
				state = STATE_START_TAG;
			}
			break;
		case STATE_START_TAG:
			if (c == '/') {
				state = STATE_BEGIN_CLOSING_TAG;
			}
			else if (!isWhitespace(c)) {
				state = STATE_READING_TAG;
				tagName = c;
			}
			break;
		case STATE_READING_TAG:
			if (c == '>') {
				HTMLElement* newElement = new HTMLElement();
				newElement->tagName = tagName;
				newElement->parentElement = currentElement;
				currentElement->children.push_back(newElement);
				elementStack.push_back(currentElement);
				currentElement = newElement;
				tagName = "";
				state = STATE_END_TAG;
			}
			else if (!isWhitespace(c)) {
				tagName += c;
			}
			break;
		case STATE_END_TAG:
			if (c == '<') {
				state = STATE_START_TAG;
			}
			else {
				currentElement->textContent += c;
			}
			break;
		case STATE_BEGIN_CLOSING_TAG:
			if (c == '>') {
				currentElement = elementStack.back();
				elementStack.pop_back();
				state = STATE_INIT;
			}
			break;
		default:
			break;
		}
	}
	return root;
}

string collectTextFromPTags(const HTMLElement* element) {
	string content;
	if (element->tagName == "p") {
		content += element->textContent + "\n";
	}
	for (const HTMLElement* child : element->children) {
		content += collectTextFromPTags(child);
	}
	return content;
}

int main(void) {
	CURL* curl;
	CURLcode res;
	string readBuffer, url, name, password;

	showIntroText();
	cout << "\nEnter your name: ";
	getline(cin, name);

	setPassword(password);

	while (password.length() <= 6) {
		cout << "Enter a password, more than 6 characters it should be: ";
		getline(cin, password);
	}

	cout << "\n\nEnter the URL for crawling: ";
	getline(cin, url);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
		}
		curl_easy_cleanup(curl);

		HTMLElement* parsedHTML = HTMLParser(readBuffer);
		string content = collectTextFromPTags(parsedHTML);

		cout << content;

		User user(name, password, content, url);


		ofstream userFile(user.name + ".txt");
		if (userFile.is_open()) {
			userFile << "User: " << user.name << "\n";
			userFile << "Password: " << user.password << "\n";
			userFile << "Content: " << user.content << "\n";
			userFile << "URL: " << user.url << "\n";
			userFile.close();
			cout << "Data written to " << user.name << ".txt" << endl;
		}
		else {
			cout << "Unable to write" << endl;
		}

		delete parsedHTML;
	}
	else {
		cerr << "Failed to initialize" << endl;
	}
	return 0;
}