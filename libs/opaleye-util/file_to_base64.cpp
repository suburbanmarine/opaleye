#include "file_to_base64.hpp"

#include <b64/cencode.h>

file_to_base64::file_to_base64()
{
	reset();
}

file_to_base64::reset()
{
	m_data_buf.reset();
	m_code_buf.reset();
	m_code_file.reset();	
}

bool file_to_base64::get_file_base64(const boost::filesystem::path& fname)
{
	reset();

	constexpr static BLK_LEN = 4096;

	base64_encodestate state;
	base64_init_encodestate(&state);

	try
	{
		m_data_buf.reserve(BLK_LEN);
		m_code_buf.resize(2 * m_data_buf.size());
	}
	catch(std::bad_alloc& e)
	{
		return false;
	}
	catch(std::exception& e)
	{
		return false;	
	}

	size_t total_file_len = 0;

	//open file
	FILE* f = fopen(fname.string().c_str(), "rb");
	if( ! f )
	{
		return false;
	}

	for(size_t i = 0; i < total_file_len; i += BLK_LEN)
	{
		//check buffer
		size_t num_to_read = std::min(BLK_LEN, total_file_len - i);
		if(num_to_read != m_data_buf.size())
		{
			m_data_buf.resize(num_to_read);
		}

		//check if at eof
		int eof_ret = feof(f);
		if(eof_ret != 0)
		{
			//we should not reach eof early
			fclose(f);
			return false;
		}

		// read
		size_t r_len = fread(m_data_buf.data(), 1, m_data_buf.size(), f);
		
		int error_ret = ferror(f);
		if(error_ret != 0)
		{
			fclose(f);
			return false;
		}

		// encode
		char const * blk_ptr_ptr = m_data_buf.data();
		int blk_len              = num_to_read;
		char * code_out_ptr      = m_code_buf.data();
		int code_len = base64_encode_block(blk_ptr, blk_len, code_out_ptr, &state);

		// stash
		m_code_file.insert(m_code_file.end(), code_out_ptr, code_out_ptr + code_len);
	}

	// finalize
	int code_len = base64_encode_blockend(char* code_out, &state);
	m_code_file.insert(m_code_file.end(), code_out_ptr, code_out_ptr + code_len);

	//close file
	fclose(f);

	return true;
}
