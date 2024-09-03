#include<stdio.h>
#include<stdlib.h>
struct node{
    int data;
    struct node*next;
};
    int linkedlisttraversal(struct node*ptr)
   {

    while(ptr != NULL)
    {
        printf("element : %d\n",ptr->data);
        ptr = ptr->next;
    }
    return 0;
   }

int main(){
    struct node*head;
    struct node*second;
    struct node*third;
    struct node*fourth;

    head = (struct node*)malloc(sizeof(struct node));
    second = (struct node*)malloc(sizeof(struct node));
    third = (struct node*)malloc(sizeof(struct node));
    fourth = (struct node*)malloc(sizeof(struct node));

    head ->data =5;
    head ->next =second;

    second ->data =10;
    second ->next =third;

    third ->data =15;
    third->next =fourth;

    fourth->data =25;
    fourth->next =NULL;

    linkedlisttraversal(head);
    return 0;

}
