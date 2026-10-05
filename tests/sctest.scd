(
if(NetAddr.langPort != 57120) {
    "Port 57120 is unavailable; stop the other SuperCollider language instance.".postln;
    1.exit;
} {

s.options.device = "default";
OSCFunc({
    s.quit;
    SystemClock.sched(0.2, { 0.exit; nil });
}, '/sendosc/test/quit', NetAddr("127.0.0.1"), recvPort: 57120);
"SENDOSC_TEST_STARTED".postln;

s.waitForBoot({
    {
        var sig = SinOsc.ar(440);
        var env = EnvGen.kr(Env.perc(0.01, 0.5), doneAction: 2);
        sig * env ! 2;
    }.play;

    x = OSCFunc({ |msg, time, addr, recvPort|
        var state = msg[1];
        ("Received state update: " ++ state).postln;
        case
        { state == 1 } {
            { SinOsc.ar(880) * EnvGen.kr(Env.perc(0.01, 0.1), doneAction: 2) ! 2 }.play;
        }
        { state == 0 } {
            { Pulse.ar(220) * EnvGen.kr(Env.perc(0.005, 0.05), doneAction: 2) ! 2 }.play;
        }
        { state == -1 } {
            { Saw.ar(110) * EnvGen.kr(Env.perc(0.01, 0.3), doneAction: 2) ! 2 }.play;
        };
    }, '/program/state', recvPort: 57120);

    "SENDOSC_TEST_READY".postln;
}, onFailure: {
    "SuperCollider audio server failed to boot.".postln;
    1.exit;
});
};
)