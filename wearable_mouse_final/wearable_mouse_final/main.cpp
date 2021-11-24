#include<iostream>
#include<Windows.h>
#include<atlstr.h>
#include<stdio.h>
#include<string.h>
#include<string>
#include "SerialPort.h"

using namespace std;


#pragma warning (disable:4996);

CSerialPort _serial;

void gotoxy(int x, int y) {

    COORD Cur;
    Cur.X = x;
    Cur.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);

}

void cursor(int n) // Ŀ�� ���̱� & �����
{
    HANDLE hConsole;
    CONSOLE_CURSOR_INFO ConsoleCursor;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    ConsoleCursor.bVisible = n;
    ConsoleCursor.dwSize = 1;

    SetConsoleCursorInfo(hConsole, &ConsoleCursor);
}


int CHAR_TO_DEC(const char* CHAR) // �ƽ�Ű�ڵ� -> int(4�ڸ�������)
{
    int len;
    int val = 0;
    len = strlen(CHAR);
    if (CHAR[0] == 43) {
        if (len == 2) {
            if (CHAR[1] < 58 && CHAR[1]>47) {
                val = val + CHAR[1] - 48;
            }
        }
        if (len == 3) {
            if (CHAR[2] < 58 && CHAR[2]>47) {
                val = val + (CHAR[2] - 48);
                val = val + (CHAR[1] - 48) * 10;
            }
        }
        if (len == 4) {
            if (CHAR[3] < 58 && CHAR[3]>47) {
                val = val + (CHAR[3] - 48);
                val = val + (CHAR[2] - 48) * 10;
                val = val + (CHAR[1] - 48) * 100;
            }
        }
        if (len == 5) {
            if (CHAR[4] < 58 && CHAR[4]>47) {
                val = val + (CHAR[4] - 48);
                val = val + (CHAR[3] - 48) * 10;
                val = val + (CHAR[2] - 48) * 100;
                val = val + (CHAR[1] - 48) * 1000;
            }
        }

    }

    if (CHAR[0] == 45) {
        if (len == 2) {
            if (CHAR[1] < 58 && CHAR[1]>47) {
                val -= CHAR[1] - 48;
            }
        }
        if (len == 3) {
            if (CHAR[2] < 58 && CHAR[2]>47) {
                val -= (CHAR[2] - 48);
                val -= (CHAR[1] - 48) * 10;
            }
        }
        if (len == 4) {
            if (CHAR[3] < 58 && CHAR[3]>47) {
                val = val - (CHAR[3] - 48);
                val = val - (CHAR[2] - 48) * 10;
                val = val - (CHAR[1] - 48) * 100;
            }
        }
        if (len == 5) {
            if (CHAR[4] < 58 && CHAR[4]>47) {
                val = val + (CHAR[4] - 48);
                val = val + (CHAR[3] - 48) * 10;
                val = val + (CHAR[2] - 48) * 100;
                val = val + (CHAR[1] - 48) * 1000;
            }
        }
    }

    return val;

}

int result[50] = { 0 }; // �������� ���� �񱳸� ����

void tokenize(BYTE* buf2) {

    char* buf = new char[50];
    for (int i = 0; i < 50; i++) {
        buf[i] = buf2[i];
    }

    const char* token[50];

    token[0] = strtok(buf, ","); // , �������� ù ��ū ����

    result[0] = CHAR_TO_DEC(token[0]);

    int i = 0, j = 0;

    for (;;) {
        //printf("%s ", token[i]);
        i++;
        token[i] = strtok(NULL, ",");
        if (token[i] == NULL) break;
        result[i] = CHAR_TO_DEC(token[i]);
        //  printf("%d ", result[i]);
    }

}

int main() {
    ///////////////////////////////// ���콺 Ŀ�� ���� �κ�
    int i;
    POINT pt; // ���콺 ������
    int x[50];
    int y[50];

    int pitch = 0;
    int roll = 0;

    int beginR = 0;
    int beginL = 0;
    int tempR = 0;
    int tempL = 0;

    int l_mouse_state[2] = { 0 };
    int r_mouse_state[2] = { 0 };

    for (i = 0; i < 50; i++) {
        x[i] = 0;
        y[i] = 0;
    }
    SetCursorPos(beginR, beginL);

    string str;

    cout << "������ ��Ʈ��ȣ�� �Է��ϼ���(�� : COM5) : ";
    cin >> str;

    CString cs(str.c_str());

    cs = L"" + cs;

    int isSuccess = 1;

    if (_serial.OpenPort(cs))   // ���� COM Port
    {
        // BaudRate, ByteSize, fParity, Parity, StopBit ���� ����
        _serial.ConfigurePort(19200, 8, FALSE, NOPARITY, ONESTOPBIT);
        // Timeout ����
        _serial.SetCommunicationTimeouts(0, 0, 0, 0, 0);

        // Buffer�� ���  
        BYTE* pByte = new BYTE[50];
 
        for (;;) {
            
            if (_serial.ReadByte(pByte, 50))
            {
                // �дµ� �����ϸ� ó���ϰ�
                if (isSuccess) {
                    cout << "���� ����" << endl;
                    isSuccess = false;
                }

                pByte[50 - 1] = '\0';
                tokenize(pByte);
                //_insertData(CString(reinterpret_cast<char*>(pByte)));

            }
            else {
                // ��� ���� �߻���
                printf("���� �߻�\n");
                break;
            }

            for (int k = 0; k < 50; k++) {
                if (result[k] > 525 && result[k] < 530) {
                    pitch = pitch - 1;
                }
                if (result[k] > 530 && result[k] < 540) {
                    pitch = pitch - 2;
                }
                if (result[k] > 540 && result[k] < 550) {
                    pitch = pitch - 5;
                }
                if (result[k] > 550 && result[k] < 560) {
                    pitch = pitch - 6;
                }
                if (result[k] > 560 && result[k] < 570) {
                    pitch = pitch - 7;
                }
                if (result[k] > 570 && result[k] < 560) {
                    pitch = pitch - 9;
                }
                if (result[k] > 580 && result[k] < 590) {
                    pitch = pitch - 10;
                }





                if (result[k] < 475 && 470 < result[k]) {
                    pitch = pitch + 1;
                }
                if (result[k] < 470 && 460 < result[k]) {
                    pitch = pitch + 2;
                }
                if (result[k] < 460 && 450 < result[k]) {
                    pitch = pitch + 5;
                }
                if (result[k] < 450 && 440 < result[k]) {
                    pitch = pitch + 7;
                }
                if (result[k] < 440 && 430 < result[k]) {
                    pitch = pitch + 8;
                }
                if (result[k] < 430 && 420 < result[k]) {
                    pitch = pitch + 9;
                }
                if (result[k] < 420 && 410 < result[k]) {
                    pitch = pitch + 10;
                }







                if (25 < result[k] && result[k] < 30) {
                    roll = roll + 1;
                }

                if (30 < result[k] && result[k] < 40) {
                    roll = roll + 2;
                }

                if (40 < result[k] && result[k] < 50) {
                    roll = roll + 3;
                }

                if (50 < result[k] && result[k] < 60) {
                    roll = roll + 5;
                }

                if (60 < result[k] && result[k] < 70) {
                    roll = roll + 6;
                }

                if (70 < result[k] && result[k] < 80) {
                    roll = roll + 9;
                if (80 < result[k] && result[k] < 90) {
                }

                    roll = roll + 10;
                }









                if (-30 < result[k] && result[k] < -25) {
                    roll = roll - 1;

                }
                if (-40 < result[k] && result[k] < -30) {
                    roll = roll - 2;
                }
                if (-50 < result[k] && result[k] < -40) {
                    roll = roll - 3;
                }

                if (-60 < result[k] && result[k] < -50) {
                    roll = roll - 7;
                }if (-70 < result[k] && result[k] < -60) {
                    roll = roll - 9;
                }

                if (-80 < result[k] && result[k] < -70) {
                    roll = roll - 10;
                }
                if (-90 < result[k] && result[k] < -80) {
                    roll = roll - 11;
                }

                if ((result[k] > 7200 && result[k] < 8200) && l_mouse_state[0] == 0 && l_mouse_state[1] == 0) {
                    // ��Ŭ�� ����->����
                    mouse_event(MOUSEEVENTF_LEFTUP, pitch, roll, 0, 0);  // ��ǥ�� �°� ���ʹ�ư ����
                    l_mouse_state[0] = 1;
                    l_mouse_state[1] = 0;
                }

                if ((result[k] < 6500 && result[k] > 5000) && l_mouse_state[0] == 1 && l_mouse_state[1] == 0) {
                    //printf("��Ŭ�� ����\n");
                    mouse_event(MOUSEEVENTF_LEFTDOWN, roll, pitch, 0, 0); // ��ǥ�� �°� ���ʹ�ư ������
                    Sleep(1);

                    l_mouse_state[0] = 0;
                    l_mouse_state[1] = 0;
                }

                if (result[k] > 3200 && result[k] < 4200 && r_mouse_state[0] == 0 && r_mouse_state[1] == 0) { // ��Ŭ�� ���� -> ����
                    r_mouse_state[0] = 1;
                    r_mouse_state[1] = 0;
                }

                if ((result[k] < 2500 && result[k] > 1000) && r_mouse_state[0] == 1 && r_mouse_state[1] == 0) {
                    // ��Ŭ��!
                    mouse_event(MOUSEEVENTF_RIGHTDOWN, roll, pitch, 0, 0);
                    Sleep(1);
                    mouse_event(MOUSEEVENTF_RIGHTUP, pitch, roll, 0, 0);
                    r_mouse_state[0] = 0;
                    r_mouse_state[1] = 0;
                }

                SetCursorPos(roll, pitch);
            }

            if (GetKeyState(VK_F6) < 0) // F6Ű ������
            {
                printf("���α׷�����\n");
                return 0; // ���α׷� ����
            }


        }

        delete[] pByte;
    }

    _serial.ClosePort();

}

//#include<Windows.h>
//#include<atlstr.h>
//#include<stdio.h>
//#include<string.h>
//#include "SerialPort.h"
//
//#pragma warning (disable:4996);
//
//CSerialPort _serial;
//
//void gotoxy(int x, int y) {
//
//    COORD Cur;
//    Cur.X = x;
//    Cur.Y = y;
//    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
//
//}
//
//void cursor(int n) // Ŀ�� ���̱� & �����
//{
//    HANDLE hConsole;
//    CONSOLE_CURSOR_INFO ConsoleCursor;
//
//    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//
//    ConsoleCursor.bVisible = n;
//    ConsoleCursor.dwSize = 1;
//
//    SetConsoleCursorInfo(hConsole, &ConsoleCursor);
//}
//
//
//int CHAR_TO_DEC(const char* CHAR) // �ƽ�Ű�ڵ� -> int(4�ڸ�������)
//{
//    int len;
//    int val = 0;
//    len = strlen(CHAR);
//    if (CHAR[0] == 43) {
//        if (len == 2) {
//            if (CHAR[1] < 58 && CHAR[1]>47) {
//                val = val + CHAR[1] - 48;
//            }
//        }
//        if (len == 3) {
//            if (CHAR[2] < 58 && CHAR[2]>47) {
//                val = val + (CHAR[2] - 48);
//                val = val + (CHAR[1] - 48) * 10;
//            }
//        }
//        if (len == 4) {
//            if (CHAR[3] < 58 && CHAR[3]>47) {
//                val = val + (CHAR[3] - 48);
//                val = val + (CHAR[2] - 48) * 10;
//                val = val + (CHAR[1] - 48) * 100;
//            }
//        }
//        if (len == 5) {
//            if (CHAR[4] < 58 && CHAR[4]>47) {
//                val = val + (CHAR[4] - 48);
//                val = val + (CHAR[3] - 48) * 10;
//                val = val + (CHAR[2] - 48) * 100;
//                val = val + (CHAR[1] - 48) * 1000;
//            }
//        }
//
//    }
//
//    if (CHAR[0] == 45) {
//        if (len == 2) {
//            if (CHAR[1] < 58 && CHAR[1]>47) {
//                val -= CHAR[1] - 48;
//            }
//        }
//        if (len == 3) {
//            if (CHAR[2] < 58 && CHAR[2]>47) {
//                val -= (CHAR[2] - 48);
//                val -= (CHAR[1] - 48) * 10;
//            }
//        }
//        if (len == 4) {
//            if (CHAR[3] < 58 && CHAR[3]>47) {
//                val = val - (CHAR[3] - 48);
//                val = val - (CHAR[2] - 48) * 10;
//                val = val - (CHAR[1] - 48) * 100;
//            }
//        }
//        if (len == 5) {
//            if (CHAR[4] < 58 && CHAR[4]>47) {
//                val = val + (CHAR[4] - 48);
//                val = val + (CHAR[3] - 48) * 10;
//                val = val + (CHAR[2] - 48) * 100;
//                val = val + (CHAR[1] - 48) * 1000;
//            }
//        }
//    }
//
//    return val;
//
//}
//
//int result[50] = { 0 }; // �������� ���� �񱳸� ����
//
//void tokenize(BYTE* buf2) {
//
//    char* buf = new char[50];
//    for (int i = 0; i < 50; i++) {
//        buf[i] = buf2[i];
//    }
//
//    const char* token[50];
//
//    token[0] = strtok(buf, "_"); // _ �������� ù ��ū ����
//
//    result[0] = CHAR_TO_DEC(token[0]);
//
//    int i = 0, j = 0;
//
//    for (;;) {
//        //printf("%s ", token[i]);
//        i++;
//        token[i] = strtok(NULL, "_");
//        if (token[i] == NULL) break;
//        result[i] = CHAR_TO_DEC(token[i]);
//        //  printf("%d ", result[i]);
//    }
//
//}
//
//void main() {
//    ///////////////////////////////// ���콺 Ŀ�� ���� �κ�
//    int i;
//    POINT pt; // ���콺 ������
//    int x[50];
//    int y[50];
//
//    int pitch = 0;
//    int roll = 0;
//
//    int beginR = 0;
//    int beginL = 0;
//    int tempR = 0;
//    int tempL = 0;
//
//    int l_mouse_state[2] = { 0 };
//    int r_mouse_state[2] = { 0 };
//
//    for (i = 0; i < 50; i++) {
//        x[i] = 0;
//        y[i] = 0;
//    }
//    SetCursorPos(beginR, beginL);
//
//
//    if (_serial.OpenPort(L"COM5"))   // ���� COM Port
//    {
//        // BaudRate, ByteSize, fParity, Parity, StopBit ���� ����
//        _serial.ConfigurePort(CBR_9600, 8, FALSE, NOPARITY, ONESTOPBIT);
//        // Timeout ����
//        _serial.SetCommunicationTimeouts(0, 0, 0, 0, 0);
//
//        // Buffer�� ���  
//        BYTE* pByte = new BYTE[50];
//        for (;;) {
//            // �дµ� �����ϸ� ó���ϰ�  
//            if (_serial.ReadByte(pByte, 50))
//            {
//                pByte[50 - 1] = '\0';
//                tokenize(pByte);
//                //_insertData(CString(reinterpret_cast<char*>(pByte)));
//
//            }
//            else {
//                // ��� ���� �߻���
//                printf("���� �߻�\n");
//                break;
//            }
//
//            for (int k = 0; k < 50; k++) {
//                if (result[k] > 530 && result[k] < 540) {
//                    pitch = pitch - 3;
//                }
//                if (result[k] > 540 && result[k] < 560) {
//                    pitch = pitch - 5;
//                }
//
//                if (result[k] < 480 && 460 < result[k]) {
//                    pitch = pitch + 3;
//                }
//
//                if (result[k] < 460 && 440 < result[k]) {
//                    pitch = pitch + 5;
//                }
//
//                if (30 < result[k] && result[k] < 40) {
//                    roll = roll + 3;
//                }
//
//                if (40 < result[k] && result[k] < 60) {
//                    roll = roll + 5;
//                }
//
//                if (-40 < result[k] && result[k] < -20) {
//                    roll = roll - 3;
//                }
//
//                if (-60 < result[k] && result[k] < -40) {
//                    roll = roll - 5;
//                }
//
//                if ((result[k] > 7200 && result[k] < 8200) && l_mouse_state[0] == 0 && l_mouse_state[1] == 0) {
//                    // ��Ŭ�� ����->����
//                    mouse_event(MOUSEEVENTF_LEFTUP, pitch, roll, 0, 0);  // ��ǥ�� �°� ���ʹ�ư ����
//                    l_mouse_state[0] = 1;
//                    l_mouse_state[1] = 0;
//                }
//
//                if ((result[k] < 5600 && result[k] > 5000) && l_mouse_state[0] == 1 && l_mouse_state[1] == 0) {
//                    //printf("��Ŭ�� ����\n");
//                    mouse_event(MOUSEEVENTF_LEFTDOWN, roll, pitch, 0, 0); // ��ǥ�� �°� ���ʹ�ư ������
//                    Sleep(1);
//
//                    l_mouse_state[0] = 0;
//                    l_mouse_state[1] = 0;
//                }
//
//                if (result[k] > 3800 && result[k] < 4500 && r_mouse_state[0] == 0 && r_mouse_state[1] == 0) { // ��Ŭ�� ���� -> ����
//                    r_mouse_state[0] = 1;
//                    r_mouse_state[1] = 0;
//                }
//
//                if ((result[k] < 2000 && result[k] > 1300) && r_mouse_state[0] == 1 && r_mouse_state[1] == 0) {
//                    // ��Ŭ��!
//                    mouse_event(MOUSEEVENTF_RIGHTDOWN, roll, pitch, 0, 0);
//                    Sleep(1);
//                    mouse_event(MOUSEEVENTF_RIGHTUP, pitch, roll, 0, 0);
//                    r_mouse_state[0] = 0;
//                    r_mouse_state[1] = 0;
//                }
//
//                SetCursorPos(roll, pitch);
//            }
//
//        }
//
//        delete[] pByte;
//    }
//
//    _serial.ClosePort();
//
//}