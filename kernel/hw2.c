#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/cred.h>
#include <linux/sched.h>

static bool check_ban_helper(char ban, char c){
	if(c == '7'){
		return 1;
	}
	if(ban == 'g'){
                return (c=='1' || c=='3' || c=='5');
        }
        if(ban== 'p'){
                return (c=='2' || c=='3' || c=='6');
        }
        if(ban == 'k'){
                return (c=='4' || c=='5' || c=='6');
        }
	return 0;  
}

static void change_ban(struct task_struct *task, int pm, char ban){
	int ban_num;
	ban_num = (ban=='g') ? 1 : (ban=='p') ? 2 : 4;
	task->ban_char += pm*ban_num;
}

asmlinkage long sys_set_ban(int ban_getpid, int ban_pipe, int ban_kill){
        int ban_num;
	char c;
	if (ban_getpid < 0 || ban_pipe < 0 || ban_kill < 0){
		return -EINVAL;
	}
	if (!uid_eq(current_uid(), GLOBAL_ROOT_UID)){
		return -EPERM;
	}
	ban_num=0;
	if(ban_getpid){
		ban_num+=1;
	}
	if(ban_pipe){
		ban_num+=2;
	}
	if(ban_kill){
		ban_num+=4;
	}
	c = '0' + ban_num;
	current->ban_char = c;
	return 0;
}

asmlinkage long sys_get_ban(char ban){
	char c;
	if(ban!='g' && ban!='p' && ban!='k'){
		return -EINVAL;
	}
	c = current->ban_char;
	return check_ban_helper(ban, c);
}

asmlinkage long sys_check_ban(pid_t pid, char ban){
	struct pid *pid_struct;
	struct task_struct *task;
	char c;
	if(ban != 'g' && ban != 'p' && ban != 'k'){
		return -EINVAL;
	}
	pid_struct = find_get_pid(pid);
	if(!pid_struct){
     		return -ESRCH;
	}
	task = get_pid_task(pid_struct, PIDTYPE_PID);
	put_pid(pid_struct);
	if(!task){
		return -ESRCH;
	}
	if(check_ban_helper(ban, current->ban_char)){
		return -EPERM;
	}
	
    res = check_ban_helper(ban, task->ban_char);
    put_task_struct(task);
	return res;
}

asmlinkage long sys_flip_ban_branch(int height, char ban){
	int num_of_flipped;
	int num_of_banned;
	struct task_struct *curr_parent;
	if(height <= 0 || (ban!='g' && ban!='p' && ban!='k')){
		return -EINVAL;
	}
	if(check_ban_helper(ban, current->ban_char)){
		return -EPERM;
	}
	num_of_flipped=0;
	num_of_banned=0;
	curr_parent = current->parent; 
	while(curr_parent && num_of_flipped < height){
		//check if already banned, if so remove ban
		if(check_ban_helper(ban, curr_parent->ban_char)){
			change_ban(curr_parent, -1, ban);
			num_of_flipped++;
		}
		//check if not banned, if so ban
		else{
			change_ban(curr_parent,  1, ban);
			num_of_flipped++;
			num_of_banned++;
		}
		//check if parent is init, if so return
		if(curr_parent->pid == 1){
			return num_of_banned;
		}
		curr_parent = curr_parent->parent;
	}
	return num_of_banned;
}
