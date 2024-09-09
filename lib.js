addToLibrary({
	save_data: function(where_cstr, ptr, size) {
		const where = UTF8ToString(where_cstr);
		const data = HEAPU8.subarray(ptr, ptr + size);
		localStorage.setItem(where, JSON.stringify(data));
	},
	load_data: function(where_cstr, dst, size) {
		const where = UTF8ToString(where_cstr);
		const where_data = localStorage.getItem(where);
		if (!where_data) {
			return ;
		}
		const data = Object.values(JSON.parse(where_data));
		let i = 0;
		for (let byte of data) {
			HEAPU8[dst + i] = byte;
			i++;
		}
	},
});
