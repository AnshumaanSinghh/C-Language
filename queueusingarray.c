#include<stdio.h>
int queue[50];
int front =-1;
int rear =-1;
void push(int arr[],int size,int val)
{
    if (rear == size-1){
        printf("queue overflow");
    }
    else if(rear == -1 && front == -1){
        front = front+1;
        rear = rear+1;
        arr[rear]=val;
    }
    else{
        rear = rear+1;
        arr[rear]=val;
    }
}
void pop(int arr[])
  {
    if(rear == -1 && front == -1){
        printf("underflow");
    }
    else{
        int element = arr[front];
        front = front+1;
        printf("popped element is :%d\n",element);
    }
  }
  void print(int arr[])
  {
    for(int i=front ;i<rear; i++){
        printf("%d",arr[i]);
    }
  }
  int main(){
    int size;
    printf("enter the size:");
    scanf("%d",&size);
    int queue[size];
    push(queue,size,30);
    push(queue,size,31);
    push(queue,size,32);
    push(queue,size,33);
    push(queue,size,34);
    pop(queue);
    print(queue);
    return 0;
  }