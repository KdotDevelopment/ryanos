#include "cstr.hpp"
#include "../memory/mem.hpp"

char uint_output[128];
const char *to_string(uint64_t value) {
	uint8_t size;
	uint64_t size_test = value;
	while(size_test / 10 > 0) {
		size_test /= 10;
		size++;
	}

	uint8_t index = 0;
	while(value / 10 > 0) {
		uint8_t remainer = value % 10;
		value /= 10;
		uint_output[size - index] = remainer + '0';
		index++;
	}
	uint8_t remainder = value % 10;
	uint_output[size - index] = remainder + '0';
	uint_output[size + 1] = 0;
	return uint_output;
}

char int_output[128];
const char *to_string(int64_t value) {
	uint8_t isNegative = 0;
	if(value < 0) {
		isNegative = 1;
		value *= -1;
		int_output[0] = '-';
	}

	uint8_t size;
	uint64_t size_test = value;
	while(size_test / 10 > 0) {
		size_test /= 10;
		size++;
	}

	uint8_t index = 0;
	while(value / 10 > 0) {
		uint8_t remainer = value % 10;
		value /= 10;
		int_output[isNegative + size - index] = remainer + '0';
		index++;
	}
	uint8_t remainder = value % 10;
	int_output[isNegative + size - index] = remainder + '0';
	int_output[isNegative + size + 1] = 0;
	return int_output;
}

char double_output[128];
const char *to_string_temp(double value, uint8_t decimal_places) {
	if(decimal_places > 20) decimal_places = 20;

	char *int_ptr = (char *)to_string((int64_t)value); //takes all the non decimal bits first
	char *double_ptr = double_output;

	if(value < 0) {
		value *= -1;
	}

	while(*int_ptr != 0) {
		*double_ptr++ = *int_ptr++;
	}

	*double_ptr = '.';
	double_ptr++;

	double new_value = value - (int)value; //leaves just decimal places

	for(uint8_t i = 0; i < decimal_places; i++) {
		new_value *= 10;
		*double_ptr = (int)new_value + '0';
		new_value -= (int)new_value;
		double_ptr++;
	}

	*double_ptr = 0;

	return double_output;
}

const char *to_string(double value, uint8_t decimal_places) {
	return to_string_temp(value, decimal_places);
}

const char *to_string(double value) {
	return to_string_temp(value, 2);
}

char hex_output[128];
const char* to_hstring(uint64_t value) {
	uint64_t* val_ptr = &value;
	uint8_t* ptr;
	uint8_t tmp;
	uint8_t size = 8 * 2 - 1;
	for (uint8_t i = 0; i < size; i++){
		ptr = ((uint8_t*)val_ptr + i);
		tmp = ((*ptr & 0xF0) >> 4);
		hex_output[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
		tmp = ((*ptr & 0x0F));
		hex_output[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
	}
	hex_output[size + 1] = 0;
	return hex_output;
}

char hex_output32[128];
const char* to_hstring(uint32_t value) {
	uint32_t* val_ptr = &value;
	uint8_t* ptr;
	uint8_t tmp;
	uint8_t size = 4 * 2 - 1;
	for (uint8_t i = 0; i < size; i++){
		ptr = ((uint8_t*)val_ptr + i);
		tmp = ((*ptr & 0xF0) >> 4);
		hex_output32[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
		tmp = ((*ptr & 0x0F));
		hex_output32[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
	}
	hex_output32[size + 1] = 0;
	return hex_output32;
}

char hex_output16[128];
const char* to_hstring(uint16_t value) {
	uint16_t* val_ptr = &value;
	uint8_t* ptr;
	uint8_t tmp;
	uint8_t size = 2 * 2 - 1;
	for (uint8_t i = 0; i < size; i++){
		ptr = ((uint8_t*)val_ptr + i);
		tmp = ((*ptr & 0xF0) >> 4);
		hex_output16[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
		tmp = ((*ptr & 0x0F));
		hex_output16[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
	}
	hex_output16[size + 1] = 0;
	return hex_output16;
}

char hex_output8[128];
const char* to_hstring(uint8_t value) {
	uint8_t* val_ptr = &value;
	uint8_t* ptr;
	uint8_t tmp;
	uint8_t size = 1 * 2 - 1;
	for (uint8_t i = 0; i < size; i++){
		ptr = ((uint8_t*)val_ptr + i);
		tmp = ((*ptr & 0xF0) >> 4);
		hex_output8[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
		tmp = ((*ptr & 0x0F));
		hex_output8[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
	}
	hex_output8[size + 1] = 0;
	return hex_output8;
}