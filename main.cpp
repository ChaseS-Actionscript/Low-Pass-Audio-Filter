#include <iostream>
#include <ios>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>
int MEM_fread(unsigned char* buf, size_t size, size_t n, unsigned char* *f) {
    memcpy(buf, *f, size * n);
    *f += size * n;
    return n;
}
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
uint16_t bytesToShort(char first, char second){
    return (uint16_t)((unsigned char)first | (unsigned char)second << 8);
}
uint32_t bytesToInt32(char first, char second, char third, char fourth){
    return (uint32_t)((unsigned char)first | 
        (unsigned char)second << 8 |
        (unsigned char)third << 16 | 
        (unsigned char)fourth << 24); 
}
void pointerArithTest(char* array, size_t size){
    uint32_t temp = (uint32_t)(array); 
    temp += sizeof(char)*(size-1);
    for(; array<(char*)temp; array+=2){
    // Every sample is a point on a wave
    // So if the point is really high it's going to be louder
    unsigned short waveSample = (unsigned short)((unsigned char)*array | (unsigned char)*(array+1) << 8); 
    waveSample*=1;
    memcpy(array, &waveSample, 2);
    }
}

void writeBackToFile(char* header, char* buffer, size_t sizeOfBuffer){
    std::string filePath = (std::filesystem::current_path() / "output.wav").string();
    std::fstream ofs(filePath.c_str(), std::ios::out | std::ios::binary);
    if (!ofs){
        std::cerr << filePath.c_str() << " Not found" << std::endl;
    }
    
    ofs.write(header, 44);
    ofs.write(buffer, sizeOfBuffer);
    
}
void writeBackToFileCsv(unsigned char* buffer, size_t sizeOfBuffer, std::string fileName){
    std::string filePath = (std::filesystem::current_path() / fileName).string();
    std::fstream ofs(filePath.c_str(), std::ios::out | std::ios::binary);
    if (!ofs){
        std::cerr << filePath.c_str() << " Not found" << std::endl;
    }
    for(int i = 0; i<sizeOfBuffer-1; i+=2){
        ofs << ((unsigned short)(buffer[i]) | (unsigned short)(buffer[i+1]) << 8) << ", ";
    }
}
int main(){

    std::string filePath = (std::filesystem::current_path() / "file.wav").string();
    std::fstream ifs(filePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (!ifs){
        std::cerr << filePath.c_str() << " Not found " << std::endl;
    }

    long length = static_cast<long>(ifs.tellg())-44;
    long lengthreal = ifs.tellg();
    //std::cout << length << std::endl;
    char* buffer = new char[length];
    ifs.seekg(0, std::ios::beg);
    char* header = new char[44];
    ifs.read(header, 44);

    ifs.read(buffer, length);
    buffer[length]='\0';


    wav_hdr wavHeader;
    wavHeader.RIFF[0] = header[0];
    wavHeader.RIFF[1] = header[1];
    wavHeader.RIFF[2] = header[2];
    wavHeader.RIFF[3] = header[3];

    wavHeader.ChunkSize = bytesToInt32(header[4], header[5], header[6], header[7]);
    
    wavHeader.WAVE[0] = header[8];
    wavHeader.WAVE[1] = header[9];
    wavHeader.WAVE[2] = header[10];
    wavHeader.WAVE[3] = header[11];

    wavHeader.fmt[0] = header[12];
    wavHeader.fmt[1] = header[13];
    wavHeader.fmt[2] = header[14];
    wavHeader.fmt[3] = header[15];

    wavHeader.Subchunk1Size = bytesToInt32(header[16], header[17], header[18], header[19]);

    wavHeader.AudioFormat = bytesToShort(header[20], header[21]);

    wavHeader.NumOfChan = bytesToShort(header[22], header[23]);

    wavHeader.SamplesPerSec = bytesToInt32(header[24], header[25], header[26], header[27]);

    wavHeader.bytesPerSec = bytesToInt32(header[28], header[29], header[30], header[31]);

    wavHeader.blockAlign = bytesToShort(header[32], header[33]);

    wavHeader.bitsPerSample = bytesToShort(header[34], header[35]);

    wavHeader.Subchunk2ID[0] = header[36];
    wavHeader.Subchunk2ID[1] = header[37];
    wavHeader.Subchunk2ID[2] = header[38];
    wavHeader.Subchunk2ID[3] = header[39];

    wavHeader.Subchunk2Size = bytesToInt32(header[40], header[41], header[42], header[43]);
    
    writeBackToFileCsv(reinterpret_cast<unsigned char*>(buffer), length, "out1.csv");
    pointerArithTest(buffer, length);
    writeBackToFile(header, buffer, length);
    writeBackToFileCsv(reinterpret_cast<unsigned char*>(buffer), length, "out2.csv");
    
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << wavHeader.Subchunk2Size;
    delete buffer;
    delete header;
    return 0;
}