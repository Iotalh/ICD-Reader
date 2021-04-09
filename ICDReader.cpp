#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class DateUtil {
private:
	const static int SECPERDAY = 86400;
	const static int YEAROFFSET = 1900;
	const static int MONOFFSET = 1;
public:
	static time_t StrtoDate(string str) {
		struct tm date = {};
		sscanf_s(str.c_str(), "%4d%2d%2d", &date.tm_year, &date.tm_mon, &date.tm_mday);
		date.tm_year -= YEAROFFSET;
		date.tm_mon -= MONOFFSET;
		return mktime(&date) / SECPERDAY;
	}
	static time_t getMonthEndDate(string sdate) {
		struct tm date = {};
		sscanf_s(sdate.c_str(), "%4d%2d%2d", &date.tm_year, &date.tm_mon, &date.tm_mday);
		date.tm_year -= 1900;
		if (date.tm_mon == 12) {
			date.tm_mon = 0;
			date.tm_year += 1;
		}
		date.tm_mday = 1;
		return (mktime(&date) / SECPERDAY) - 1;
	}
};

class Record {
private:
	string ID;
	time_t sdate;
	time_t edate;
	string ICD;

public:
	Record() {}
	Record(string ID, time_t sdate, time_t edate, string ICD)
		: ID(ID), sdate(sdate), edate(edate), ICD(ICD) {}
	// 將一行紀錄根據 ICD 拆分成多個 Record
	static vector<Record> parseLine(string str) {
		stringstream ss(str);
		string t;
		vector<string> data;
		vector<Record> records;
		while (getline(ss, t, ',')) {
			data.push_back(t);
		}
		time_t end_date = (data[2] == "") ? DateUtil::getMonthEndDate(data[1]) : DateUtil::StrtoDate(data[2]);
		for (size_t i = 3; i < data.size(); i++) {
			if (data[i] != "") {
				records.push_back(Record(data[0], DateUtil::StrtoDate(data[1]), end_date, data[i]));
			}
		}
		return records;
	}

	bool operator<(const Record& right) {
		if (ID != right.ID) {
			return ID < right.ID;
		}
		if (ICD != right.ICD) {
			return ICD < right.ICD;
		}

		if (sdate != right.sdate) {
			return sdate < right.sdate;
		}
		return edate < right.edate;
	}
	bool hasSameIDAndICD(Record r) { return this->ID == r.ID && this->ICD == r.ICD; }
	friend class HDayCalculator;
};

class Node {
public:
	Node* next;
	Record record;
	Node() {}
	Node(Record r) : record(r), next(nullptr) {}
};

class List {
private:
	Node* head;
public:
	List() {
		head = new Node;
	};
	void insert(Record r) {
		Node* newNode = new Node(r);
		for (Node* i = head; i != nullptr; i = i->next)
		{
			if (i->next == nullptr) {
				i->next = newNode;
				break;
			}
			else if (newNode->record < i->next->record) {
				newNode->next = i->next;
				i->next = newNode;
				break;
			}

		}
	}
	void push_front(Record r) {
		Node* newNode = new Node(r);
		newNode->next = head->next;
		head->next = newNode;
	}

	Node* getFirst() { return head->next; }
};

class HDayCalculator {
private:
	Node* start, * end;
	int days;
	time_t maxDate(time_t left, time_t right) {
		return (left > right) ? left : right;
	}

	void calDays() {
		time_t previousEndDate = 0;
		for (Node* i = start; i != end; i = i->next)
		{
			Record cur = i->record;
			if (previousEndDate < cur.sdate) {
				days += cur.edate - cur.sdate + 1;
			}
			else if (previousEndDate >= cur.sdate && previousEndDate <= cur.edate) {
				days += cur.edate - previousEndDate;
			}
			previousEndDate = maxDate(previousEndDate, cur.edate);
		}
	}

public:
	HDayCalculator(Node* start, Node* end) : start(start), end(end), days(0) {
		calDays();
	}
	void printDays() {
		cout << start->record.ID << "," << start->record.ICD << "," << days << endl;
	}
};

int main(int argc, char* argv[]) {
	ifstream fin;
	fin.open(argv[1]);
	string str;
	List list;
	while (getline(fin, str)) {
		vector<Record> rs;
		rs = Record::parseLine(str);
		for (size_t i = 0; i < rs.size(); i++) {
			list.insert(rs[i]);
		}
	}
	Node* start = list.getFirst(), * end;
	for (Node* i = list.getFirst(); ; i = i->next) {
		if (i != nullptr && i->record.hasSameIDAndICD(start->record)) {
			continue;
		}
		else {
			end = i;
			HDayCalculator hdc(start, end);
			hdc.printDays();
			start = i;
		}
		if (i == nullptr) { break; }
	}

	fin.close();
}