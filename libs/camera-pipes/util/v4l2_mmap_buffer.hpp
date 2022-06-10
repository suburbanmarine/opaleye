#pragma once

#include "opaleye-util/errno_util.hpp"

#include "util/v4l2_util.hpp"

#include <memory>

class v4l2_mmap_buffer_base
{
public:
	v4l2_mmap_buffer_base();
	virtual ~v4l2_mmap_buffer_base();

    uint32_t get_index() const;         // mmap buffer index
    uint32_t get_width() const;         // px width
    uint32_t get_height() const;        // px height
    uint32_t get_bytes_per_line() const;
    uint32_t get_pixel_format() const;  // format code

    uint32_t get_bytes_used() const  // number of bytes with actual data within the larger mmap area
    {
    	return m_buf.bytesused;
    }

	const v4l2_buffer& get_buf() const
	{
		return m_buf;
	}
	const v4l2_format& get_fmt() const
	{
		return m_fmt;
	}

	void set_buf(const v4l2_buffer& buf)
	{
		m_buf = buf;
	}

	v4l2_buffer m_buf;
	v4l2_format m_fmt;
	size_t      m_idx;
protected:
	errno_util m_errno;
};

class v4l2_mmap_buffer : public v4l2_mmap_buffer_base
{
public:
	v4l2_mmap_buffer();
	~v4l2_mmap_buffer() override;
	bool init(const int fd, const v4l2_buffer& buf, const v4l2_format& fmt, const size_t idx);
	bool unmap();

    void* get_data() const;  // mmap buffer
    size_t get_size() const; // mmap size

    //reset buf except for idx and type, ready to be enqueued
    void reset_buf();

	std::shared_ptr<std::vector<uint8_t>> copy_to_vec() const;

protected:
	void*  m_mmap_ptr;
	size_t m_mmap_size;
};

class v4l2_mmap_mp_buffer : public v4l2_mmap_buffer_base
{
public:
	v4l2_mmap_mp_buffer();
	~v4l2_mmap_mp_buffer() override;
	bool init(const int fd, const v4l2_buffer& buf, const v4l2_format& fmt, const size_t idx);
	bool unmap();

    void* get_data() const;  // mmap buffer
    size_t get_size() const; // mmap size

    //reset buf ecept for idx and type, ready to be enqueued
    void reset_buf();

	std::shared_ptr<std::vector<std::vector<uint8_t>>> copy_to_vec_mp() const;

protected:
	std::vector<v4l2_plane> m_planes;

	std::vector<void*>  m_mmap_ptr;
	std::vector<size_t> m_mmap_size;
};
