/*************************************************************************************************
sendosc -- Copyright (c) 2021, Markus Iser, KIT - Karlsruhe Institute of Technology

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

#include "sendosc/sendosc.h"

int main(int argc, const char** argv) {
    OSC::Stream osc("127.0.0.1", 5000);
    osc << OSC::Message("/tes") << OSC::Float(0.5) << OSC::Flush();
    osc << OSC::Message("/test") << OSC::Int(42) << OSC::Flush();
    osc << OSC::Message("/test") << OSC::String("abrakadabra") << OSC::Int(42) << OSC::Flush();
    osc << OSC::Message("/test") << OSC::String("abrakadabra") << OSC::Message("/test2") << OSC::Int(42) << OSC::Flush();
    osc << OSC::Message("/test") << OSC::String("abrakada") << OSC::Int(42) << OSC::Flush();
    osc << OSC::Message("/test") << OSC::String("abrakad") << OSC::Int(42) << OSC::Flush();
}
