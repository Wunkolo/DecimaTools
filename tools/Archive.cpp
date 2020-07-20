#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <Decima/Decima.hpp>

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}


	auto Archive = Decima::Archive::OpenArchive(argv[1]);
	if( !Archive ) return EXIT_FAILURE;

	Decima::Archive::FileHeader Header = Archive->GetHeader();
	std::printf(
		"Version:             %12.08X\n"
		"Key:                 %12.08X\n"
		"FileSize:            %12lu\n"
		"DataSize:            %12lu\n"
		"FileTableCount:      %12lu\n"
		"ChunkTableCount:     %12u\n"
		"MaxChunkSize:        %12u\n",
		std::uint32_t(Header.Version),
		Header.Key,
		Header.FileSize,
		Header.DataSize,
		Header.FileTableCount,
		Header.ChunkTableCount,
		Header.MaxChunkSize
	);

	Archive->IterateFileEntries(
		[](const Decima::Archive::FileEntry& CurEntry)
		{
			std::printf(
				"\t---------------------------------------\n"
				"\tFileID:        %24.u\n"
				"\tUnknown04:     %24.u\n"
				"\tUnknown08:     %24.016lX\n"
				"\tSpan.Offset:   %24lu\n"
				"\tSpan.Size:     %24u\n"
				"\tSpan.Hash:     %24.08X\n",
				CurEntry.FileID,
				CurEntry.Unknown04,
				CurEntry.Unknown08,
				CurEntry.Span.Offset,
				CurEntry.Span.Size,
				CurEntry.Span.Hash
			);
		}
	);

	Archive->IterateChunkEntries(
		[](const Decima::Archive::ChunkEntry& CurChunk)
		{
			std::printf(
				"\t---------------------------------------------\n"
				"\tUncompressed.Offset:  %24.lu\n"
				"\tUncompressed.Size:    %24.u\n"
				"\tUncompressed.Hash:    %24.08X\n"
				"\tCompressed.Offset:    %24.lu\n"
				"\tCompressed.Size:      %24.u\n"
				"\tCompressed.Hash:      %24.08X\n",
				CurChunk.UncompressedSpan.Offset,
				CurChunk.UncompressedSpan.Size,
				CurChunk.UncompressedSpan.Hash,
				CurChunk.CompressedSpan.Offset,
				CurChunk.CompressedSpan.Size,
				CurChunk.CompressedSpan.Hash
			);
		}
	);

	return 0;
}