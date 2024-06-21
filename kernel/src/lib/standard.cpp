#include "standard.hpp"

int compare_string(const char s1[], const char s2[]) {
	int i;
	for(i = 0; s1[i] == s2[i]; i++) {
		if(s1[i] == '\0') return 0;
	}
	return s1[i] - s2[i];
}

int string_length(char s[]) {
	int i = 0;
	while(s[i] != '\0') i++;
	return i;
}

void append(char s[], char n) {
	int len = string_length(s);
	s[len] = n;
	s[len + 1] = '\0';
}

char *strcopy(char *d, const char *s) {
	char *saved = d;
	while (*s) {
		*d++ = *s++;
	}
	*d = 0;
	return saved;
}