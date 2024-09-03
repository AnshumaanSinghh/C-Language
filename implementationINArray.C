#include<stdio.h>

void display(int arr[],int n){
    for(int i =0 ;i<n; i++)
    {
        printf("%d",arr[i]);
    }
    printf("\n");
}
 int indInsertion(int arr[],int size,int element,int capacity,int index)
{
    if(size>=capacity)
    {
        return -1;
    }
    for(int i =size-1;i>=index;i--)
    {
        arr[i+1] = arr[i];
        arr[index]= element;
        return 1;
    }
    return 0;

}

int main(){
    int arr[100] = {1 , 2 , 3 , 4};
    int size = 4,element = 8,index = 3;
    display(arr,size);
    indInsertion(arr, size, element,100,index);
    size += 1;
    display(arr , size);
    return 0;
    
}
/*
void display(int arr[],int n){
    for (int i = 0; i < n; i++)
    {
        printf("%d",arr[i]);
    }
    printf("\n");
}
int deletion(int arr[],int size,int index)
{
    if(size==-1)
    {return -1; }
    
    for(int i = index;i <= size-1;i++)
    {
        arr[i]=arr[i + 1];
    }
}
int main(){
    int arr[10]={1,2,3,4,5};
    int size=10,index=9;
    display(arr,size);
    deletion(arr,size,index);
    size -= 1;
    display(arr,size);
}

*/