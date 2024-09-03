#include<stdio.h>

int stack[20];
int top = -1;
void print(int arr[])
{
    for(int i=0;i<=top;i++)
    {
        printf("elemnt of stack:%d\n",arr[i]);
    }
}
void push (int arr[],int size,int value){
    if (top == size-1){
        printf("overflow");
    }
    else{
         top=top+1;
        arr[top]=value;
       
    }
}
 
void pop(int arr[]){
    if(top == -1)
    {
        printf("underflow");
        }
    else{
        int element = arr[top];
        top = top-1;
        printf("popped element is :%d\n",element);
    }
}
int main()
{
    int size;
    printf("enter size of stack:");
    scanf("%d",&size);
    

    push(stack,size,55);
    push(stack,size,56);
    push(stack,size,57);
    pop(stack);
    print(stack);
    return 0;
}