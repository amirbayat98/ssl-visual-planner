#ifndef WRITEPROTO_H
#define WRITEPROTO_H

#include "../../ssl-playbook.pb.h"

class WriteProto
{
public:
    WriteProto();

    Plans plan;

    void fillAgentInitPos(double x, double y);
};

#endif // WRITEPROTO_H
