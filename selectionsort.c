#include<stdio.h>
int main(){
    int arr[8]={15,7,9,3,1,4,8,6};
    int n=8;
    for(int i=0;i<n;i++){
        printf("%d \t",arr[i]);
    }
   for(int i=0;i<n-1;i++){
    int min=arr[i];
    int mindx=i;
    for(int j=i;j<n;j++){
        if(min>arr[j]){
            min=arr[j];
            mindx=j;
        }
    }
    int temp=arr[mindx];
    arr[mindx]=arr[i];
    arr[i]=temp;
   } 
   printf("\n");
   for(int i=0;i<n;i++){
    printf("%d \t",arr[i]);
   }
    return 0;
}