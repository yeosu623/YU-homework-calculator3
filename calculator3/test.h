#pragma once
#include <string>
#include <map>
#include <iostream>
#include <cctype>
#include <vector>
using namespace std;

//Term 과제. >,<,>=,<= 관계 연산자가 동작하도록 수정한 코드
bool prev_inequality_sign = false;
vector<double> value_log;
vector<char> inequality_log;
void inequality() {
	// 값 설정
	char sign = inequality_log[0];
	double a = value_log[0];
	double b = value_log[1];

	//기능 구현
	switch (sign) {
	case '>':
		if (a > b) { cout << a << ">" << b << " is True" << endl; break; }
		else { cout << a << ">" << b << " is False" << endl; break; }
	case '<':
		if (a < b) { cout << a << "<" << b << " is True" << endl; break; }
		else { cout << a << "<" << b << " is False" << endl; break; }
	case ']': // a>=b
		if (a >= b) { cout << a << ">=" << b << " is True" << endl; break; }
		else { cout << a << ">=" << b << " is False" << endl; break; }
	case '[': // a<=b
		if (a <= b) { cout << a << "<=" << b << " is True" << endl; break; }
		else { cout << a << "<=" << b << " is False" << endl; break; }
	}

	//초기화
	prev_inequality_sign = false;
	value_log.clear();
	inequality_log.clear();
};

enum class Kind : char
{
	name, number, end,
	plus = '+', minus = '-', mul = '*', div = '/', print = ';', assign = '=', lp = '(', rp = ')'
};

struct Token
{
	Kind kind;
	string string_value;
	double number_value;
};

double expr(bool get);
double term(bool get);
double prim(bool get);

map<string, double> table;

int no_of_errors;
double error(const string& s)
{
	no_of_errors++;
	cerr << "error: " << s << '\n';
	return 1;
}

class Token_stream
{
public:
	Token_stream(istream& s) : ip{ &s }, owns{ false } {}
	Token_stream(istream* p) : ip{ p }, owns{ true } {}
	~Token_stream() { close(); }

	Token get()
	{
		//char ch = 0;
		//*ip >> ch;
		char ch;
		do {
			if (!ip->get(ch))
				return ct = { Kind::end };
		} while (ch != '\n' && isspace(ch));

		switch (ch)
		{
		case 0:
			return ct = { Kind::end };

		case '*':
		case '/':
		case '+':
		case '-':
		case '(':
		case ')':
		case '=':
			if (prev_inequality_sign == true) {
				if (inequality_log.back() == '>') {
					inequality_log.pop_back(); inequality_log.push_back(']');
					return ct = { Kind::print }; // prim에 아무것도 없는 것을 방지
				}
				else if (inequality_log.back() == '<') {
					inequality_log.pop_back(); inequality_log.push_back('[');
					return ct = { Kind::print }; // prim에 아무것도 없는 것을 방지
				}
			}
			else
				return ct = { static_cast<Kind>(ch) };
		case '>':
			inequality_log.push_back('>');
			prev_inequality_sign = true;
			return ct = { static_cast<Kind>(ch) };
		case '<':
			inequality_log.push_back('<');
			prev_inequality_sign = true;
			return ct = { static_cast<Kind>(ch) };

		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case '.':
			ip->putback(ch);
			*ip >> ct.number_value;
			ct.kind = Kind::number;
			return ct;

		case ';':
		case '\n':
			return ct = { Kind::print };

		default:
			if (isalpha(ch))
			{
				ct.string_value = ch;
				while (ip->get(ch) && isalnum(ch))
					ct.string_value += ch;
				ip->putback(ch);
				ct.kind = Kind::name;
				return ct;
			}

			error("bad token");
			return ct = { Kind::print };
		}
	}
	const Token& current() { return ct; }

	void set_input(istream& s) { close(); ip = &s; owns = false; }
	void set_input(istream* p) { close(); ip = p; owns = true; }

private:
	void close() { if (owns) delete ip; }
	istream* ip;
	bool owns;
	Token ct{ Kind::end };
};

Token_stream ts{ cin };

double prim(bool get)
{
	if (get)
		ts.get();

	switch (ts.current().kind)
	{
	case Kind::number:
	{
		double v = ts.current().number_value;
		ts.get();
		return v;
		break;
	}

	case Kind::name:
	{
		double& v = table[ts.current().string_value];
		if (ts.get().kind == Kind::assign)
			v = expr(true);
		return v;
	}

	case Kind::minus:
		return -prim(true);

	case Kind::lp:
	{
		auto e = expr(true);
		if (ts.current().kind != Kind::rp)
			return error("')' expected");
		ts.get();
		return e;
	}

	default:
		return error("primary expected");
	}
}

double term(bool get)
{
	double left = prim(get);
	for (;;)
	{
		switch (ts.current().kind)
		{
		case Kind::mul:
			left *= prim(true);
			break;

		case Kind::div:
			if (auto d = prim(true))
			{
				left /= d;
				break;
			}
			return error("divide by 0");

		default:
			return left;
		}
	}
}

double expr(bool get)
{
	double left = term(get);
	for (;;)
	{
		switch (ts.current().kind)
		{
		case Kind::plus:
			left += term(true);
			break;

		case Kind::minus:
			left -= term(true);
			break;
		default:
			return left;
		}
	}
}

void calculate()
{
	for (;;)
	{
		ts.get();
		if (ts.current().kind == Kind::end) break;
		if (ts.current().kind == Kind::print) continue;
		auto r = expr(false);
		cout << r << endl;
		if (inequality_log.size() >= 1) {
			value_log.push_back(r);
			if (value_log.size() >= 2) {
				inequality();
			}
		}
	}
}
