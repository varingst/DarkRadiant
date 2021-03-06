#pragma once

#include "iarchive.h"
#include "stream/FileInputStream.h"

namespace archive
{

/// \brief An ArchiveFile which is stored as a single file on disk.
class DirectoryArchiveFile :
	public ArchiveFile
{
private:
	std::string _name;
	stream::FileInputStream _istream;
	stream::FileInputStream::size_type _size;

public:
	typedef stream::FileInputStream::size_type size_type;

	DirectoryArchiveFile(const std::string& name, const std::string& filename) :
		_name(name),
		_istream(filename)
	{
		if (!failed())
		{
			_istream.seek(0, stream::FileInputStream::end);
			_size = _istream.tell();
			_istream.seek(0);
		}
		else 
		{
			_size = 0;
		}
	}

	bool failed() const
	{
		return _istream.failed();
	}

	size_type size() const override
	{
		return _size;
	}

	const std::string& getName() const override
	{
		return _name;
	}

	InputStream& getInputStream() override
	{
		return _istream;
	}
};

}