package main

import (
	"bufio"
	"fmt"
	"os"
	"sync"
	"time"
)

// 创建一个从标准输入读取的 reader
var reader = bufio.NewReader(os.Stdin)

func readRune() (r rune, err error) {
	r, _, err = reader.ReadRune()
	return r, err
}

const (
	ArraySize  = 5
	InputSize  = 25
	MaxThreads = 6
)

type DataStructure struct {
	data  [ArraySize]int
	valid bool
}

var (
	A_register, B_register, C_even_register, C_odd_register DataStructure
	A_iregister, B_iregister, C_iregister                   DataStructure
	D_register                                              DataStructure
	D_even_iregister, D_odd_iregister                       DataStructure
	input_data                                              [InputSize]int
	program_terminated                                      bool
	mutex_D                                                 sync.Mutex
	threadReady, clockSignal                                chan struct{}
)

func init() {
	input_data = [InputSize]int{2, 3, 18, 9, 1, 34, 52, 11, 26, 8, 12, 44, 8, 97, 10, 22, 31, 4, 6, 48, 5, 0, 13, 49, 7}
	threadReady = make(chan struct{}, MaxThreads)
	clockSignal = make(chan struct{}, 1)
}

func function_A() {
	index := 0
	for !program_terminated {
		<-clockSignal
		for i := 0; i < ArraySize; i++ {
			A_iregister.data[i] = input_data[index%InputSize]
			index++
		}
		A_register = A_iregister
		A_register.valid = true
		fmt.Printf("First element of structure at \nA:%d B:%d  C1:%d C2:%d D:%d\n", A_register.data[0], B_register.data[0], C_odd_register.data[0], C_even_register.data[0], D_register.data[0])
		threadReady <- struct{}{}
	}
}

func function_B() {
	for !program_terminated {
		<-clockSignal
		if !A_register.valid {
			continue
		}
		for i := 0; i < ArraySize; i++ {
			B_iregister.data[i] = A_register.data[i] - 1
		}
		A_register.valid = false
		B_register = B_iregister
		B_register.valid = true
		threadReady <- struct{}{}
	}
}

func function_C() {
	for !program_terminated {
		<-clockSignal
		if !B_register.valid {
			continue
		}
		for i := 0; i < ArraySize; i++ {
			C_iregister.data[i] = B_register.data[i] - 1
		}
		B_register.valid = false
		if C_iregister.data[1]%2 == 0 {
			C_even_register = C_iregister
			C_even_register.valid = true
		} else {
			C_odd_register = C_iregister
			C_odd_register.valid = true
		}
		threadReady <- struct{}{}
	}
}

func control_and_clocks() {
	var ch, chr rune
	ch, _ = readRune()
	for !program_terminated {
		if ch == 'p' || ch == 'P' {
			chr, _ = readRune()
			for {
				time.Sleep(100 * time.Millisecond)
				clockSignal <- struct{}{}
				if chr == 's' || chr == 'S' {
					program_terminated = true
					break
				}
			}
			break
		}
		if ch == 'l' || ch == 'L' {
			for {
				chr, _ = readRune()
				if chr == '\n' {
					time.Sleep(100 * time.Millisecond)
					clockSignal <- struct{}{}
				} else if chr == 's' || chr == 'S' {
					program_terminated = true
					clockSignal <- struct{}{}
					break
				}
			}
			break
		}
	}
}

func function_D_even() {
	for !program_terminated {
		<-clockSignal
		if !C_even_register.valid {
			continue
		}
		for i := 0; i < ArraySize; i++ {
			D_even_iregister.data[i] = C_even_register.data[i] - 1 // decrement by 1 for even values
		}
		C_even_register.valid = false
		C_odd_register.valid = false
		mutex_D.Lock() // Lock the mutex
		D_register = D_even_iregister
		D_register.valid = true
		mutex_D.Unlock() // Unlock the mutex
		threadReady <- struct{}{}
	}
}

func function_D_odd() {
	for !program_terminated {
		<-clockSignal
		if !C_odd_register.valid {
			continue
		}
		for i := 0; i < ArraySize; i++ {
			D_odd_iregister.data[i] = C_odd_register.data[i] + 1 // increment by 1 for odd values
		}
		C_odd_register.valid = false
		C_even_register.valid = false
		mutex_D.Lock() // Lock the mutex
		D_register = D_odd_iregister
		D_register.valid = true
		mutex_D.Unlock() // Unlock the mutex
		threadReady <- struct{}{}
	}
}

func main() {
	go function_A()
	go function_B()
	go function_C()
	go control_and_clocks()
	go function_D_even()
	go function_D_odd()

	for {
		if len(threadReady) == MaxThreads {
			for i := 0; i < MaxThreads; i++ {
				<-threadReady
			}
			clockSignal <- struct{}{}
		}
	}
}
