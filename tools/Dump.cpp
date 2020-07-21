#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>

#include <Decima/Decima.hpp>

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}

	const std::filesystem::path ArchivePath(argv[1]);
	const std::filesystem::path OutputPath(ArchivePath.parent_path() / ArchivePath.stem());
	auto Archive = Decima::Archive::OpenArchive(ArchivePath);
	if( !Archive ) return EXIT_FAILURE;

	std::filesystem::create_directory(
		OutputPath
	);

	Decima::Archive::FileHeader Header = Archive->GetHeader();
	std::printf(
		"Version:             %12.08X\n"
		"Key:                 %12.08X\n"
		"FileSize:            %12lu\n"
		"DataSize:            %12lu\n"
		"FileTableCount:      %12lu\n"
		"SegmentTableCount:   %12u\n"
		"MaxSegmentSize:      %12u\n",
		std::uint32_t(Header.Version),
		Header.Key,
		Header.FileSize,
		Header.DataSize,
		Header.FileTableCount,
		Header.SegmentTableCount,
		Header.MaxSegmentSize
	);

	Archive->IterateFileEntries(
		[&Archive, &OutputPath](const Decima::Archive::FileEntry& CurEntry)
		{
			std::printf(
				"\t---------------------------------------\n"
				"\tFileID:        %24.08X\n"
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
			char FileName[32];
			std::snprintf(FileName, 32, "%08X.bin", CurEntry.FileID);
			if( std::ofstream OutputFile(OutputPath / FileName); OutputFile )
			{
				Archive->ExtractFile(CurEntry, OutputFile);
			}
		}
	);

	return 0;
}