#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include <Decima/Decima.hpp>
#include <mio/mmap.hpp>

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}

	mio::ummap_source FileMapping(argv[1]);

	Decima::FileHeader Header;

	Header = *reinterpret_cast<const Decima::FileHeader*>(FileMapping.data());
	std::printf(
		"Magic:				%08X\n"
		"Version:			%08X\n"
		"FileSize:			%12lu\n"
		"DataSize:			%12lu\n"
		"FileTableCount:	%12lu\n"
		"ChunkTableCount:	%12u\n"
		"MaxChunkSize:		%12u\n",
		Header.Magic,
		Header.Version,
		Header.FileSize,
		Header.DataSize,
		Header.FileTableCount,
		Header.ChunkTableCount,
		Header.MaxChunkSize
	);
	return 0;
}