</br>

### <div align=center> 시스템 프로그래밍 팀 이름 : [미정]</div>
</br>

## arduino_code 폴더
* UCLA 아두이노 코드 c파일로 변경하여 저장되어 있는 폴더 
* 2개의 소스코드가 있으니 코딩 시 참고 가능하다.
</br>

## Python Code 폴더
* 홈페이지에서 제공해주는 파이썬 코드 중 필요한 코드들만 있는 폴더
* 하드웨어 컨트롤 및 Opencv를 활용한 예제, 그리고 qr인식과 같은 비전 관련 폴더로 이루어져 있다. 전체 코드 대신 참고할만한 코드 일부만 선택적으로 가져왔다.
</br>

## gpio_i2c_test 폴더
* 실제 하드웨어를 구동시키기 위한 c언어 버전의 테스트 코드가 저장되어 있는 폴더
* 다음과 같이 make를 통해 컴파일이 가능하다. (원하는 컴파일을 위해선 Makefile을 적절히 구성해야 한다.)
<pre><code>make</code></pre>
* 컴파일된 파일들을 지우고 싶다면 make clean을 이용할 수 있다.
<pre><code>make clean</code></pre>
</br>

## main 폴더
* 실제 과제 수행을 위해 구동될 여러 소스 코드 및 필요 라이브러리들이 있는 폴더
+ ~~프로그래밍 시작을 위한 main.c 소스코드의 기본 구현은 5월 22일(수)까지 업데이트 예정~~
+ 5월 20일(월) 업데이트 사항 : main.c 소스코드의 기본 구현 완료
</br>

## main.c 프로그래밍을 위한 기본 정보
1. tracking sensor의 input은 검정 선을 감지할 경우 0, 흰색 배경을 감지할 경우 1이다.
2. void line_tracer()외의 코드는 수정할 일이 없을 것이다. (무슨 코드인지 알고 하는 것이 아니라면 기존 코드는 수정하지 않는 것을 추천한다.)
3. 자신이 필요한 함수가 있다면 편의에 따라 구현해서 사용하는 것을 적극 권장한다.
4. 외부 라이브러리를 사용하는 경우 알려줘야 한다. (가끔 linking문제가 발생할 수 있기 때문에 Makefile을 수정해야 하는 경우가 발생할 수 있기 때문이다.)
5. 이외의 모든 질문 사항 및 디버깅 도움은 단톡에 요청하면 된다. 만약 main.c의 구현이 늦어질 것 같으면 미루지 말고 내게 헬프를 치면 내가 다 해결해줄 것이다.
</br>

## opencv_qr 폴더
* opencv c++를 이용한 qr코드 인식을 위한 소스 코드 및 라이브러리들이 있는 폴더
* 해당 보드에 설치되어있는 opencv 버전은 4.5.0이다.
* 5월 20일(월) 업데이트 사항 : qr인식해서 인터넷 주소 받아오는 코드 및 Makefile 구현 완료
* 이 때 Makefile에 linking 옵션을 통해 opencv4 라이브러리 경로를 지정해줘야 컴파일이 가능하다. ~~(이미 해결한 문제긴 함)~~
</br>

## 편의 사항
* 라즈베리파이 shell의 ~/.bashrc 설정 파일에서 alias를 통해 git clone이 일괄적으로 처리될 수 있게 환경을 구성하였다.
* 다음 명령어를 이용해 git clone 및 버전 관리를 용이하게 할 수 있다.
<pre><code>git_folder_fresh</code></pre>
* main함수 컴파일까지 한번에 하고 싶다면 아래의 명령어를 사용할 수 있다.
<pre><code>git_folder_main</code></pre>
</br>

## 컴파일
* 과제1에 해당하는 main폴더의 main.c와 과제2에 해당하는 opencv_qr 폴더의 qr_recognition.cpp 모두 make를 이용한 컴파일을 지원한다.
<pre><code>make</code></pre>
* 컴파일된 파일들을 지우고 싶다면 make clean을 이용할 수 있다.
<pre><code>make clean</code></pre>
* 이외의 컴파일 옵션은 각 소스코드 폴더의 Makefile을 수정하여 적용할 수 있다.
</br>

## Reference
* GPIO핀에 대한 하드웨어 정보 (http://www.yahboom.net/xiazai/Raspbot/Download/Hardware%20interface%20manual.xlsx)
* 통신 프로토콜에 대한 정보 (http://www.yahboom.net/xiazai/Raspbot/Download/Pi_MCU_communication_protocol.xlsx)
* 공식 사이트 (http://www.yahboom.net/study/Raspbot)
* Opencv 재설치 관련 영상 (https://www.youtube.com/watch?v=55en3YQtE8s)
* Opencv 재설치 관련 블로그 (https://make.e4ds.com/make/learn_guide_view.asp?idx=116)
<br/>
 
#### ※ 해당 repository의 사전에 협의되지 않은 자의 코드 무단 도용 및 사용은 금지한다. 
#### ※ 악의적인 목적의 사용(치팅, 코드 무단 사용, 부정 행위 등)이 발견되거나 감지될 시 해당 사용자에 대한 불이익이 발생할 수 있음을 고지한다.
</br>

#### Updated by JH Choi on May 21, 2024
