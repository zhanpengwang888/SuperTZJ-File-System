#include <iostream>
#include <string>
#include "test_util.h"
using namespace std;

int get_index_by_offset(int offset) {
	int entry_num = 128;
	int d_limit = 10;
	int i_limit = d_limit + 4 * entry_num;
	int i2_limit = i_limit + entry_num * entry_num;
	int i3_limit = i2_limit + entry_num * entry_num * entry_num;
	int index = offset - 1;
	int b_index, f_index, s_index;
	int f_offset, s_offset;
	if(offset <= 0) {
		cout << "No nonpositive offset!" << endl;
		return -1;
	}
	else if(offset > 0 && offset <= d_limit) {
		cout << "index is" << index << endl;
		return 1;
	}
	else if(offset > d_limit && offset <= i_limit) {
		int i_elem = index - d_limit; //know the exact position in indirect block list
		f_index = i_elem/entry_num;
		f_offset = i_elem%entry_num;
		//b_index = get_index(f_node->iblocks[i_elem/entry_num],i_elem%entry_num);
		cout << "f_index is " << f_index << " f_offset is " << f_offset << endl;
		return 1;
	}
	else if(offset > i_limit && offset <= i2_limit) {
		int i2_elem = index - i_limit; //only consider in the i2 block's entry
		//get the first level indirect block index stroed in second level
		f_index = i2_elem/entry_num;
		f_offset = i2_elem%entry_num;
		cout << "f_index is " << f_index << " f_offset is " << f_offset << endl;
		return 1;
	}
	else if(offset > i2_limit && offset < i3_limit) {
		int i3_elem = index - i2_limit;
		s_index = i3_elem/(entry_num*entry_num); //find the second indirect block index in third level indirect block
		s_offset = i3_elem%(entry_num*entry_num);
		f_index = s_offset/entry_num;
		f_offset = s_offset%entry_num;
		cout << " s_index is " << s_index << " s_offset is " << s_offset << " f_index is " << f_index << " f_offset is " << f_offset << endl;
		return 1;
	}
	else {
		cout << "Invalid offset number, please check it again" <<endl;
		return -1;
	}
}

int main() {
	int entry_num = 128;
	int d_limit = 10;
	int i_limit = d_limit + 4 * entry_num;
	int i2_limit = i_limit + entry_num * entry_num;
	int i3_limit = i2_limit + entry_num * entry_num * entry_num;
	get_index_by_offset(i2_limit + 50000);
	return 0;
}