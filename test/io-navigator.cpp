#include "brickstest.hpp"

#include <bricks/io/filestream.h>
#include <bricks/io/streamreader.h>
#include <bricks/io/streamwriter.h>

using namespace Bricks;
using namespace Bricks::IO;

TEST(BricksIoNavigatorTest, EmptyFile) {
	FileStream stream(TestPath.Combine("data/empty.bin"), FileOpenMode::Open, FileMode::ReadOnly);
	StreamReader reader(tempnew stream);
	EXPECT_THROW(reader.ReadInt32(), EndOfStreamException) << "StreamReader did not throw on end of stream";
}

TEST(BricksIoNavigatorTest, WriteReadTest) {
	String path = "/tmp/libbricks-test.bin";
	const String testString = "ohai";
	const u32 testValue1 = 0x1337BAAD;
	const u16 testValue2 = 0xF33D;
	{ FileStream stream(path, FileOpenMode::Create, FileMode::WriteOnly, FilePermissions::OwnerReadWrite);
	StreamWriter writer(tempnew stream, Endian::BigEndian);
	writer.WriteInt(testValue1);
	writer.WriteString(testString);
	writer.WriteByte('\0');
	writer.WriteInt(testValue2, Endian::LittleEndian);
	EXPECT_EQ(sizeof(testValue1) + sizeof(testValue2) + testString.GetLength() + 1, stream.GetLength()) << "Written file size does not match"; }

	{ FileStream rstream(path, FileOpenMode::Open, FileMode::ReadOnly);
	StreamReader reader(tempnew rstream, Endian::LittleEndian);
	u32 num = reader.ReadInt32(Endian::BigEndian);
	EXPECT_EQ(testValue1, num);
	String str = reader.ReadString();
	EXPECT_EQ(testString, str);
	u16 num2 = reader.ReadInt16();
	EXPECT_EQ(testValue2, num2);
	EXPECT_TRUE(reader.IsEndOfFile()) << "We did not read the entire file"; }

	Filesystem::GetDefault()->DeleteFile(path);
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
