#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>



int memory_open(struct inode *inode, struct file *filp);
int memory_release(struct inode *inode, struct file *filp);
ssize_t memory_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t memory_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

static struct file_operations fops = {
        .read = memory_read,
        .write = memory_write,
        .open = memory_open,
        .release = memory_release
};

struct semaphore full;
struct semaphore mutex;

char buffer[20];
int buff_size = 0;
int init_module(void)
{
	  int result;  
	  result = register_chrdev(100, "maziar", &fops);		//register my (charachter) device to ghazie ;)
	  if (result < 0)
	  {					//nemitune major number ro assign kone be devicemoon
	  	printk("cannot obtain major number \n");
		return result;
	  }
	  sema_init(&full,1);		//semaphor ro init mikone (dar vaghe hamoon mutex init e)
	  printk("Module has been attached to kernel...\n");		//radife modulo endakht var e dele kernel
	  return 0;
}
void cleanup_module(void){
	unregister_chrdev(100, "maziar");      
	printk(KERN_INFO "Module has been removed from var e dele kernel...\n");
}

int memory_open(struct inode *inode, struct file *flip)		// harvaght ba device kar darim in function e baz mishe
{
	if(flip->f_mode & FMODE_READ)							//fmod mod e baz kardan e file e ke and mishe ba inke ma mikhaym chikar konim ba file
		if(iminor(inode))
		{
			printk("can't read from this file!\n");
			return -1;
		}
 	if(flip->f_mode & FMODE_WRITE)							//fmod mod e baz kardan e file e ke and mishe ba inke ma mikhaym chikar konim ba file
 		if(!iminor(inode))
 		{
			printk("can't write to this file!%d\n",iminor(inode));
			return -1;
		}
	return 0;
}
ssize_t memory_read(struct file *file, char *buf,size_t count,loff_t *f_pos)		//read
{
	int i,j;
	if(!buff_size)		//age chiizi va3 khoondan nabood
	{
		up(&mutex);
		return 0;	
	}
	
	i = 0;
	while( i < count )		//age bood va3 khoondan mikhoone (mibare be user mode) count uni e ke karbar khaaste
	{
		if(i >= buff_size)
			break;
		copy_to_user(buf + i,(buffer+i),1);
		i++;
	}
	down(&mutex);			//global va3 inke hamzaman nakhoone o nanevise
	
	if(i >= buff_size)		//age kolle buffer khaali shod sizesho 0 mikonim
		buff_size = 0;
	else{					//age hanuz kamel khali nashode bood fragmentetion ro az bein mibare (be tedad e khoonde shode ja be jaa mikone)
		j = i;
		while(j < 20){
			buffer[j-i] = buffer[j];
			buffer[j] = 0;
			j++;
		}
		buff_size = buff_size - i;
	}		
	up(&full);				//vaghti mikhoone dge full ro bayad bekeshe biroon azash signal kone
	up(&mutex);
   	return i;
}
ssize_t memory_write(struct file *file, const char *buf,size_t count,loff_t *f_pos){
	int i;
	i = 0;
	int arg1, arg2;
	while(i < count){					// count tedad chizaE e ke neveshte shode (mikhad user benevise)
		if(!(buff_size < 20))			//age jaa nadashtim vayse (bozorg taraz 20)
			down(&full);
		arg1 = buffer+buff_size;		//bi mored
		arg2 = buf+i;					//bi mored
		copy_from_user((buffer + buff_size), (buf + i), 1);	//copy from kernel to user space
		i = i + 1;
		down(&mutex);		//2 nafar naran too e buffsize benevisan
		buff_size += 1;		
		up(&mutex);
	}
  	return i;
}

int memory_release(struct inode *inode, struct file *flip)
{
	return 0;
}

