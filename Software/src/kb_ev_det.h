#ifndef __KB_EV_DET_H__
#define __KB_EV_DET_H__

// 用于检测键盘所在事件的编号
#define KB_EV_DET 0x01 // 键盘事件检测
#define MAX_EV_NUM 32  // 最大事件数量

struct kb_ev_det_result_t
{
    int event_num;
    int value;
};

int numOfEvents();
struct kb_ev_det_result_t kbEventDetect();
void kb_Event_Detect(int event_num);

#endif