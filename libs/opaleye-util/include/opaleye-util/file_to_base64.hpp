#pragma once

class file_to_base64
{
public:
	file_to_base64();
	
	void reset();

	bool encode_file_base64(const boost::filesystem::path& fname);	

	const std::deque<char>& get_code() const
	{
		return m_code_file;
	}

protected:
	std::vector<char> m_data_buf;
	std::vector<char> m_code_buf;
	std::deque<char>  m_code_file;
};