#pragma once

#include <boost/filesystem/path.hpp>

#include <cstddef>

class Path_util
{
public:
	static size_t num_elements(const boost::filesystem::path& path)
	{
		return std::distance(path.begin(), path.end());
	}

	static bool trailing_element_is_dir(const boost::filesystem::path& path)
	{
		return path.rbegin()->string() == ".";
	}

	// returns true if base_path is a parent path or identical to req_path
	static bool is_parent_path(const boost::filesystem::path& base_path, const boost::filesystem::path& req_path)
	{
		if( base_path.empty() )
		{
			throw std::domain_error("path must not be empty");
		}

		if( req_path.empty() )
		{
			throw std::domain_error("path must not be empty");
		}

		if( ! base_path.is_absolute() )
		{
			throw std::domain_error("must be absolute path");
		}

		if( ! req_path.is_absolute() )
		{
			throw std::domain_error("must be absolute path");
		}

		//base path must be equal to or shorter than req_path
		if(base_path.size() > req_path.size())
		{
			return false;
		}

		const bool base_path_ends_in_dir = trailing_element_is_dir(base_path);

		auto b_it = base_path.begin();
		auto r_it = req_path.begin();
		for(b_it = base_path.begin(), r_it = req_path.begin(); b_it != base_path.end(); ++b_it, ++r_it)
		{
			//if we end in a dir, are at end of base_path, and have matched so far, we match
			if(base_path_ends_in_dir && (std::next(b_it) == base_path.end()))
			{
				return true;
			}

			//if this segment does not match, we do not match
			if(*b_it != *r_it)
			{
				return false;
			}
		}

		return true;
	}
};
