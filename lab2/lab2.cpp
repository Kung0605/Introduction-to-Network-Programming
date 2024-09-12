#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <sstream>
#include <netinet/udp.h>
#include <iostream>
typedef struct {
    uint64_t magic;     /* 'BINFLAG\x00' */
    uint32_t datasize;  /* in big-endian */
    uint16_t n_blocks;  /* in big-endian */
    uint16_t zeros;
} __attribute((packed)) binflag_header_t;
typedef struct {
    uint32_t offset;        /* in big-endian */
    uint16_t cksum;         /* XOR'ed results of each 2-byte unit in payload */
    uint16_t length;        /* ranges from 1KB - 3KB, in big-endian */
    uint8_t  payload[0];
} __attribute((packed)) block_t;
typedef struct {
   uint16_t length;        /* length of the offset array, in big-endian */
   uint32_t offset[0];     /* offset of the flags, in big-endian */
} __attribute((packed)) flag_t;
int main() {
    std::string id;
    std::cin >> id;
    std::string id_cmd = "wget \"https://inp.zoolab.org/binflag/challenge?id=" + id + "\" -O challenge.bin";
    std::cout << id_cmd << std::endl;
    system(id_cmd.c_str());
    system("sleep 1s");
    std::ifstream file;
    file.open("challenge.bin", std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        perror("Open failed");
        return 1;
    }
    binflag_header_t header; 
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    header.n_blocks = ntohs(header.n_blocks);
    header.datasize = ntohl(header.datasize);
    header.zeros = ntohs(header.zeros);
    // std::cout << header.n_blocks << std::endl;
    // std::cout << header.datasize << std::endl;
    // std::cout << header.zeros << std::endl;
    block_t block;
    std::vector<uint8_t> D(header.datasize);
    for (int i = 0; i < header.n_blocks; ++i) {
        file.read(reinterpret_cast<char*>(&block), sizeof(block));
        block.offset = ntohl(block.offset);
        block.cksum = ntohs(block.cksum);
        block.length = ntohs(block.length);
        // std::cout << "offset :" << block.offset << std::endl;
        // std::cout << "cksum :" << block.cksum << std::endl;
        // std::cout << "length :" << block.length << std::endl;
        uint8_t payload[block.length];
        file.read(reinterpret_cast<char*>(payload), block.length);
        uint16_t xored;
        xored = (payload[0] << 8) | payload[1];
        for (int i = 2; i < block.length; i += 2) {
            uint16_t tmp;
            tmp = (payload[i] << 8) | payload[i + 1];
            xored = xored ^ tmp;
        }
        if (xored != block.cksum) {
            //std::cout << "wrong ckcum" << std::endl;
            continue;
        }
        for (int i = 0; i < block.length; ++i) {
            D[block.offset + i] = payload[i];
        }
    }
    flag_t flag;
    file.read(reinterpret_cast<char*>(&flag), sizeof(flag));
    flag.length = ntohs(flag.length);
    std::string answer;
    for (int i = 0; i < flag.length; ++i) {
        uint32_t tmp;
        file.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
        int offset = ntohl(tmp);
        std::stringstream ss1;
        ss1 << std::hex << (int)D[offset]; // int decimal_value
        std::string res1(ss1.str());
        answer += res1.length() == 1 ? "0" + res1 : res1;
        //std::cout << res1 << std::endl;
        std::stringstream ss2;
        ss2 << std::hex << (int)D[offset + 1]; // int decimal_value
        std::string res2(ss2.str());
        answer += res2.length() == 1 ? "0" + res2 : res2;
        //std::cout << res2 << std::endl;
    }
    // for (int i = 0; i < flag.length; ++i) {
    //     uint32_t tmp;
    //     file.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
    //     int offset = ntohl(tmp);
    //     answer += D[offset];
    //     answer += D[offset + 1];
    // }
    std::cout << "flag :" << answer << std::endl;
    std::string verifymsg = "wget \"https://inp.zoolab.org/binflag/verify?v=" + answer + "\"";
    std::cout << verifymsg << std::endl;
    system(verifymsg.c_str());
    file.close();
    return 0;
}