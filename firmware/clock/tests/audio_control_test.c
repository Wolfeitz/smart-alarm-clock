#include "audio_control.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    uint32_t test=audio_control_token();
    assert(!audio_control_is_alarm(test)&&audio_control_valid(test));
    audio_control_alarm(false);assert(audio_control_valid(test));
    audio_control_alarm(true);uint32_t alarm=audio_control_token();
    assert(audio_control_is_alarm(alarm)&&!audio_control_valid(test));
    audio_control_alarm(true);assert(audio_control_valid(alarm));
    audio_control_alarm(false);
    assert(!audio_control_valid(alarm)&&!audio_control_valid(test));
    uint32_t next_test=audio_control_token();
    assert(!audio_control_is_alarm(next_test)&&audio_control_valid(next_test));
    /* Trigger and dismissal both happen before the next audio block. */
    audio_control_alarm(true);audio_control_alarm(false);
    assert(!audio_control_valid(next_test));
    /* Repeated owner updates must not interrupt a current sound test. */
    next_test=audio_control_token();
    for(unsigned i=0;i<1000;i++)audio_control_alarm(false);
    assert(audio_control_valid(next_test));
    puts("PASS audio cancellation: test preemption, queued stale test, snooze/dismiss, rapid on/off and unchanged owner updates");
}
