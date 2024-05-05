#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <cassert>

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

class HTMLElement {
public:
	string tagName;
	vector<HTMLElement*> children;
	HTMLElement* parentElement;
	string textContent;
};

enum State {
	STATE_INIT,
	STATE_START_TAG,
	STATE_READING_TAG,
	STATE_READING_ATTRIBUTES,
	STATE_END_TAG,
	STATE_BEGIN_CLOSING_TAG
};

bool isWhitespace(char c) {
	return c == ' ';
}

HTMLElement* HTMLParser(const string& body) {
	HTMLElement* root = new HTMLElement();
	HTMLElement* currentElement = root;
	State state = STATE_INIT;
	string tagName = "";
	string text = "";

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
				if (tagName == "p") {
					currentElement = new HTMLElement();
					currentElement->tagName = tagName;
					currentElement->parentElement = root;
					root->children.push_back(currentElement);
				}
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
				if (currentElement && currentElement->tagName == "p") {
					currentElement->textContent += c;
				}
			}
			break;
		case STATE_BEGIN_CLOSING_TAG:
			if (c == '>') {
				currentElement = root;
				state = STATE_INIT;
			}
			break;
		default:
			break;
		}
	}
	return root;
}


void printTextFromPTags(const HTMLElement* element) {
	if (element->tagName == "p") {
		cout << element->textContent << endl;
	}
	for (const HTMLElement* child : element->children) {
		printTextFromPTags(child);
	}
}
std::string extractBody(const std::string& html) {
	size_t startPos = html.find("<body>");
	size_t endPos = html.find("</body>");

	if (startPos == std::string::npos || endPos == std::string::npos) {
		return ""; // Return empty string if tags are not found
	}


	startPos += 6; // Length of "<body>"

	if (endPos <= startPos) {
		return "";
	}

	return html.substr(startPos, endPos - startPos);
}


int main(void)
{
	CURL* curl;
	CURLcode res;
	string readBuffer, url, name, password;

	cout << "Enter your name: ";
	getline(cin, name);

	cout << "Enter password: ";
	getline(cin, password);

	cout << "Enter the URL for crawling: ";
	getline(cin, url);

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // Only for testing; remove in production

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
		}
		curl_easy_cleanup(curl);

		string html = readBuffer;
		//regex pattern("<p[^>]*>(.*?)</p>");
		//regex r("")
		//regex pattern("[^>]*(?=<p)|<\/p>[^<]*(?=<p|$)");

		//cout << regex_replace(html, pattern, "") << endl;


		//string body = extractBody(html);


		HTMLElement* parsedHTML = HTMLParser(html);

		printTextFromPTags(parsedHTML);

		//cout << html << endl;
		cout << "Your name" << name;
		cout << "password" << password;
	}
	else {
		cerr << "Failed to initialize cURL." << endl;
	}
	return 0;
}


