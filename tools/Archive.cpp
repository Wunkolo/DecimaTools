#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vector>

#include <Decima/Decima.hpp>
#include <mio/mmap.hpp>

#include <immintrin.h>

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}

	mio::ummap_source FileMapping(argv[1]);

	Decima::FileHeader Header;

	Header = *reinterpret_cast<const Decima::FileHeader*>(FileMapping.data());
	Header.Decrypt();

	std::printf(
		"Magic:               %12.08X\n"
		"Version:             %12.08X\n"
		"FileSize:            %12lu\n"
		"DataSize:            %12lu\n"
		"FileTableCount:      %12lu\n"
		"ChunkTableCount:     %12u\n"
		"MaxChunkSize:        %12u\n",
		Header.Magic,
		Header.Version,
		Header.FileSize,
		Header.DataSize,
		Header.FileTableCount,
		Header.ChunkTableCount,
		Header.MaxChunkSize
	);


	//// File Entries
	std::vector<Decima::FileEntry> FileEntries;
	FileEntries.resize(Header.FileTableCount);
	std::memcpy(
		FileEntries.data(),
		FileMapping.data() + sizeof(Decima::FileHeader),
		sizeof(Decima::FileEntry) * Header.FileTableCount
	);

	for(Decima::FileEntry& CurEntry : FileEntries)
	{
		CurEntry.Decrypt();
		std::printf(
			"\t---------------------------------------\n"
			"\tEntryID:       %24.u\n"
			"\tUnknown04:     %24.u\n"
			"\tUnknown08:     %24.016lX\n"
			"\tOffset:        %24lu\n"
			"\tSize:          %24u\n"
			"\tUnknown1C:     %24.08X\n",
			CurEntry.EntryID,
			CurEntry.Unknown04,
			CurEntry.Unknown08,
			CurEntry.Offset,
			CurEntry.Size,
			CurEntry.Unknown1C
		);
	}
	

	// Chunk Entries

	std::vector<Decima::ChunkEntry> ChunkEntries;
	ChunkEntries.resize(Header.ChunkTableCount);
	std::memcpy(
		ChunkEntries.data(),
		FileMapping.data()
		+ sizeof(Decima::FileHeader)
		+ sizeof(Decima::FileEntry) * Header.FileTableCount,
		sizeof(Decima::ChunkEntry) * Header.ChunkTableCount
	);

	for(Decima::ChunkEntry& CurChunk : ChunkEntries)
	{
		CurChunk.Decrypt();
		std::printf(
			"\t---------------------------------------------\n"
			"\tOffsetUncompressed:  %24.lu\n"
			"\tSizeUncompresed:     %24.u\n"
			"\tUnknown0C:           %24.08X\n"
			"\tOffsetCompressed:    %24.lu\n"
			"\tSizeCompressed:      %24.u\n"
			"\tUnknown1C:           %24.08X\n",
			CurChunk.OffsetUncompressed,
			CurChunk.SizeUncompresed,
			CurChunk.Unknown0C,
			CurChunk.OffsetCompressed,
			CurChunk.SizeCompressed,
			CurChunk.Unknown1C
		);
	}


	return 0;
}