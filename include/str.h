#ifndef CUSTOM_STR_H
#define CUSTOM_STR_H

/*extern char isNum(char c);
extern char isAlpha(char c);

extern void skip_whitespace(char ** str);
extern void skipi_whitespace(const char * str, int * i);*/

/* Define Functions */

char isNum(char c) {
	return !(c < '0' || c > '9');
}

char isNumFull(char c) {
	return !(c < '0' || c > '9') || c == ',' || c == '-';
}

char isAlpha(char c) {
	return !(c < 'a' || c > 'z') || !(c < 'A' || c > 'Z');
}

char is_alphanum(char c) {
	return isAlpha(c) || isNum(c);
}

void skip_whitespace(char ** str) {
	char ws = 1;
	while(ws) {
		switch(**str) {
			case '\t':
			case ' ':
			//case '\n':
			case '\v':
			case '\f':
			case '\r':
				(*str)++;
				break;
			default:
				ws = 0;
				break;
		}
	}
}
void skipi_whitespace(const char * str, int * i) {
	char ws = 1;
	while(ws) {
		switch(*str) {
			case '\t':
			case ' ':
			//case '\n':
			case '\v':
			case '\f':
			case '\r':
				(*i)++;
				break;
			default:
				ws = 0;
				break;
		}
	}
}

void skipToWS(char ** str) {
	char ws = 0;
	while(!ws) {
		switch(**str) {
			case '\t':
			case ' ':
			case '\n':
			case '\v':
			case '\f':
			case '\r':
			case '\0':
				ws = 1;
				(*str)--;
				break;
			default:
				break;
		}
		(*str)++;
	}
}

float fstr(char** data) {
	float n = 0.f;
	char dec = 0, neg = 0;
	
	skip_whitespace(data);
	
	while(**data != '\n' && **data != '\0') {
		switch(**data) {
			case '-':
				neg = 1;
				break;
			case '.':
				dec = 0;
				break;
			case 'f':
				break;
			default:
				n = n * 10 + (**data-'0');
				dec++;
				break;
		}
		(*data)++;
	}
	for(int i = 0; i < dec; i++) {
		n /= 10;
	}
	n = (neg)? -n:n;
	return n;
}

int istr(char** data) {
	int n = 0;
	char neg = 0;
	while(**data != '\n' && **data != '\0') {
		switch(**data) {
			case '-':
				neg = 1;
				break;
			default:
				n = n * 10 + **data-'0';
				break;
		}
		(*data)++;
	}
	return (neg)? -n:n;
}

#endif
