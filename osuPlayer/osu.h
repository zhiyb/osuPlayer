#pragma once

namespace osu
{
	using namespace concurrency;
	using namespace Platform;
	using namespace Windows::Foundation;
	using namespace Windows::Storage;
	using namespace Windows::Storage::Streams;

	unsigned int readULEB128FromReader(DataReader^ reader);
	String^ readStringFromReader(DataReader^ reader);

	public ref class osuBeatmap sealed
	{
	public:
		osuBeatmap(DataReader^ reader);
	};

	public ref class osuDB sealed
	{
	public:
		static IAsyncOperation<osuDB^>^ loadFromFileAsync(StorageFile^ file);

		property int version {int get() { return _version; }};
		property int folderCount {int get() { return _folderCount; }};
		property bool unlocked {bool get() { return _unlocked; }};
		property String^ playerName {String^ get() { return _playerName; }};
		property int bmapCount {int get() { return _bmapCount; }};

	private:
		int _version;
		int _folderCount;
		bool _unlocked;
		String^ _playerName;
		int _bmapCount;
	};

	public ref class osu sealed
	{
	public:
		osu() : _db(nullptr) {}
		static IAsyncOperation<osu^>^ loadFromFolderAsync(StorageFolder^ folder);

		property osuDB^ db {osuDB^ get() { return _db; }};

	private:
		osuDB^ _db;
	};
}
