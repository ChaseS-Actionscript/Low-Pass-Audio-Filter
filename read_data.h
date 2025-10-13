struct int24{
    uint32_t data : 24;
};
typedef struct WAV_HEADER{
    char                RIFF[4];        // RIFF Header      Magic header
    uint32_t       ChunkSize;      // RIFF Chunk Size  
    char                WAVE[4];        // WAVE Header      
    char                fmt[4];         // FMT header       
    uint32_t       Subchunk1Size;  // Size of the fmt chunk                                
    unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 
    unsigned short      NumOfChan;      // Number of channels 1=mono 2=stereo                   
    uint32_t       SamplesPerSec;  // Sampling Frequency in Hz                             
    uint32_t       bytesPerSec;    // bytes per second 
    unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo 
    unsigned short      bitsPerSample;  // Number of bits per sample      
    char                Subchunk2ID[4]; // "data"  string   
    uint32_t       Subchunk2Size;  // Sampled data length    
}wav_hdr;
template <typename sampleSize>
void readWaveData(char* array, size_t size);

template<>
void readWaveData<int24>(char* array, size_t size);