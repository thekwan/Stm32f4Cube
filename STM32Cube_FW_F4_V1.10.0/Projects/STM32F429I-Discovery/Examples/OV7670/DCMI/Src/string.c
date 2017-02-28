//#include <stddef.h>
#include <stdint.h>
#include "string.h"

void hex2str(char *str, uint8_t data) {
	uint8_t lsb = data & 0xF;
	uint8_t msb = (data>>4) & 0xF;

	str[0] = '0';
	str[1] = 'x';

	if( msb < 0xA )
		str[2] = msb + 0x30;
	else
		str[2] = msb + 55;

	if( lsb < 0xA )
		str[3] = lsb + 0x30;
	else
		str[3] = lsb + 55;

	str[4] = 0x0;

	return;
}

void hex2str_32b(char *str, int data) {
	uint8_t hex;
	int i;

	*str++ = '0';
	*str++ = 'x';

	for( i = 7 ; i >= 0 ; i-- ) {
		hex = (data >> (i*4)) & 0xF;
		if( hex < 0xA )
			*str++ = hex + 0x30;
		else
			*str++ = hex + 55;
	}

	*str = 0x0;

	return;
}

int str2hex(const char *str, uint8_t *data, int strMaxLength) {
	int i = 0;
	int ret = 0;

	*data = 0;
	while( strMaxLength-- ) {
		char c = str[i++];
		*data <<= 4;
		if( c >= 0x61 )
			*data += (c - 0x61) + 0xA;
		else if( c >= 0x41 )
			*data += (c - 0x41) + 0xA;
		else if( c >= 0x30 )
			*data += (c - 0x30);
		else
			ret = -1;	// invalid character
	}

	return ret;
}


int embed_strncmp(const char *str1, const char *str2, int length) {
	while( length-- > 0 ) {
		if( *str1++ != *str2++ )
			return 0;
	}

	return 1;
}

static char *saved_ptr;
char* embed_strtok(char *str, const char *delim) {
	int i = 0, j = 0;
	
	if( str != NULL )
		saved_ptr = str;
	else
		str = saved_ptr;

	while( str[i] != 0x0 && delim[j] != 0x0 ) {
		if( str[i++] == delim[j] ) {
			j++;
		}
	}

	if( delim[j] == 0x0 ) {
		str[i-j] = 0x0;
		saved_ptr = &str[i];
		return str;
	} else {
		saved_ptr = NULL;
		return NULL;
	}
}

int embed_strnlen(const char *str, int max_length) {
	int i;

	for( i = 0 ; i < max_length ; i++ ) {
		if( *str++ == 0x0 )
			break;
	}

	return i;
}


