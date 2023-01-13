#ifndef __GDT_H
#define __GDT_H

#include "types.h"

class GlobalDescriptorTable{

    public:
        class SegementDescriptor{
            private:
                uint16_t limit_lo;
                uint16_t base_lo;
                uint8_t base_hi;
                uint8_t type;
                uint8_t flags_limit_hi;
                uint8_t base_vhi;
            public:
                SegementDescriptor(uint32_t limit, uint32_t base, uint8_t type);
                uint32_t Base();
                uint32_t Limit();

        } __attribute__((packed));

        SegementDescriptor nullSegmentDescriptor;
        SegementDescriptor unusedSegmentDescriptor;
        SegementDescriptor codeSegmentDescriptor;
        SegementDescriptor dataSegmentDescriptor;

        GlobalDescriptorTable();
        ~GlobalDescriptorTable();

        uint16_t CodeSegmentSelector();
        uint16_t DataSegmentSelector();
};


#endif
