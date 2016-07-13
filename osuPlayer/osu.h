#pragma once

namespace osu
{
	using namespace concurrency;
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation;
	using namespace Windows::Storage;
	using namespace Windows::Storage::Streams;

	using std::vector;

	typedef std::pair<int, double> idPair;
	value struct timingPoint
	{
		double bpm;
		double offset;		// ms
		bool nonInherited;
	};

	value struct osuBeatmap
	{
		String ^artist, ^title, ^creator;
		String ^difficulty;
		String ^audioFile, ^hash, ^osuFile;
		int dummy;
		int8 ranked;
		int16 hitcircles, sliders, spinners;
		int64 modTime;
		float approachRate, circleSize, hpDrain, overallDifficulty;
		double sliderVelocity;
		idPair rating[4];
		int32 drainTime;	// s
		int32 totalTime;	// ms
		int32 previewTime;	// ms
		Vector<timingPoint>^ timingPoints;
		int32 bmapID, bmapSetID, threadID;
		int8 grade[4];
		int16 localOffset;
		float leniency;
		int8 mode;
		String ^source, ^tags;
		int16 onlineOffset;
		String ^titleFont;
		bool unplayed;
		int64 lastPlayTime;
		bool osz2;
		String ^folder;
		int64 checkedTime;
		bool ignoreSounds, ignoreSkin;
		bool disableSB, disableVideo, visualOverride;
		int16 unknown;
		int32 lastModTime;
		char maniaSpeed;
	};

	unsigned int readULEB128FromReader(DataReader^ reader);
	String^ readStringFromReader(DataReader^ reader);
	void readBeatmapFromReader(DataReader^ reader, osuBeatmap *bmap);

	public ref class osuDB sealed
	{
	public:
		static IAsyncOperation<osuDB^>^ loadFromFileAsync(StorageFile^ file);

		property int Version {int get() { return version; }};
		property int FolderCount {int get() { return folderCount; }};
		property bool Unlocked {bool get() { return unlocked; }};
		property String^ PlayerName {String^ get() { return playerName; }};
		property int BeatmapCount {int get() { return bmapCount; }};
		property Vector<osuBeatmap *>^ Beatmaps {Vector<osuBeatmap *>^ get() { return bmaps; }}

	private:
		~osuDB() { for (auto bmap : bmaps) delete bmap; }

		int version;
		int folderCount;
		bool unlocked;
		String^ playerName;
		int bmapCount;
		Vector<osuBeatmap *>^ bmaps;
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
