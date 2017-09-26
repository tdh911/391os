#include "fs.h"

#define BUFLEN 40000
//pointers to start of file system blocks
static boot_block_head_t* boot_block;
static dentry_t* dentries;
static inode_t* inodes;
static data_block_t* data_blocks;
//array of which dentry_t have a string of size 32
static uint32_t max_string[MAX_DENTRY];

static void check_for_max();
static int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
static int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
static int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* void fs_init
 * inputs: pointer to the start of the file system in memory
 * outputs: none
 * side effects: sets all global pointers to start of corresponding blocks
 * function: initializes file system variables for further use
 */
void fs_init(uint8_t* fs_img){
    //pointer to beginning of fs img
    boot_block = (boot_block_head_t*)fs_img;

    //set pointers to beginning of their spots
    dentries = (dentry_t*)(fs_img + DENTRY_SIZE);
    inodes = (inode_t*)(fs_img + BLOCK_SIZE);
    data_blocks = (data_block_t*)(fs_img + BLOCK_SIZE + boot_block->num_inodes*BLOCK_SIZE);

    //figure out what dentries have strings of size 32
    check_for_max();
}

/* void check_for_max
 * inputs: none
 * outputs: none
 * side effects: fills in max_string array with 1 for a dentry with fname size 32
                or 0 with fname size <32
 * function: check all dentries to see what fnames are max length
 */
void check_for_max(){
    uint32_t i,j,max,num_de;
    uint8_t *dstr;
    //get number of dentries
    num_de = boot_block->num_dir_entries;
    for(i = 0; i < num_de; i++){
        max = 0;
        //get fname
        dstr = (uint8_t*)(dentries[i].fname);
        //check fname for \0, if exists, then length < 32
        for(j = 0; j < FNAME_LEN; j++){
            if(dstr[j] == '\0'){
                max_string[i] = 0;
                max = 1;
                break;
            }
        }
        //no \0 found, string must be length 32
        if(max == 0){
            max_string[i] = 1;
        }
    }
}

/* int32_t read_dentry_by_name
 * inputs: const uint8_t* fname - string of file name
            dentry_t* dentry - pointer to dentry to be filled in
 * outputs: int32_t - 0 for success, -1 for failure
 * side effects: fills in given dentry pointer with copy of dentry to be searched for
 * function: sees if dentry exists with fname, if so, copy dentry structure
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    uint32_t i,len,num_de;
    len = strlen((int8_t*)fname);
    num_de = boot_block->num_dir_entries;

    //if name is too large, truncate it
    if(len > FNAME_LEN)
        len = FNAME_LEN;

    for(i = 0; i < num_de; i++){
        //if string of this dentry is length 32
        if(max_string[i]){
            if(FNAME_LEN == len && strncmp((int8_t*)fname,dentries[i].fname,len) == 0){
                //if match, fill in dentry and return success
                strncpy(dentry->fname,dentries[i].fname,FNAME_LEN);
                dentry->ftype = dentries[i].ftype;
                dentry->inode_num = dentries[i].inode_num;
                return 0;
            }
        }
        //its safe to call strlen
        else{
            if(strlen(dentries[i].fname) == len && strncmp((int8_t*)fname,dentries[i].fname,len) == 0){
                //if match, fill in dentry and return success
                strncpy(dentry->fname,dentries[i].fname,FNAME_LEN);
                dentry->ftype = dentries[i].ftype;
                dentry->inode_num = dentries[i].inode_num;
                return 0;
            }
        }
    }
    //no dentry found with matching name
    return -1;
}

/* int32_t read_dentry_by_index
 * inputs: uint32_t index - index of dentry
            dentry_t* dentry - pointer to dentry to be filled in
 * outputs: int32_t - 0 for success, -1 for failure
 * side effects: fills in given dentry pointer with copy of dentry to be searched for
 * function: gets dentry with that index, fills in given dentry with copy
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    //if index is out of bounds
    if(index >= boot_block->num_dir_entries)
        return -1;

    //copy dentry info over
    strncpy(dentry->fname,dentries[index].fname,FNAME_LEN);
    dentry->ftype = dentries[index].ftype;
    dentry->inode_num = dentries[index].inode_num;

    return 0;
}

/* int32_t read_data
 * inputs: uint32_t inode - inode number to get data
            uint32_t offset - number of bytes to offset from start
            uint8_t* buf - char buffer to fill
            uint32_t lenght - number of bytes to read
 * outputs: int32_t -  number of bytes written to buffer, -1 for failure
 * side effects: fills in given buffer with chars read from file system
 * function: reads file system given inode, returns data to buffer
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t start, db_num, db_idx, counter,len;
    inode_t *i_ptr;

    //invalid inode
    if(inode >= boot_block->num_inodes)
        return -1;
    //get pointer to inode
    i_ptr = &inodes[inode];
    //get length of file
    len = i_ptr->len;
    //offset is outside of range of file
    if(offset >= len)
        return 0;

    //pick the right data block with offset
    db_idx = offset/BLOCK_SIZE;
    db_num = i_ptr->db[db_idx];

    //data block DNE
    if(db_num >= boot_block->num_data_blocks)
        return -1;

    //get index to correct blocks
    start = offset % BLOCK_SIZE;

    counter = 0;
    //keep filling in until buffer is filled with length chars
    while(counter != length && counter+offset < len){
        buf[counter] = data_blocks[db_num].data[start];
        //if reach the end of the block, go to next block
        if(start == BLOCK_SIZE - 1){
            db_idx++;
            db_num = i_ptr->db[db_idx];
            //if the next data block is invalid
            if(db_num >= boot_block->num_data_blocks)
                return counter;
            //reset pointers
            start = 0;
        }
        //increment normally
        else
            start++;
        counter++;
    }
    return counter;
}


/* void print_all_files
 * inputs: none
 * outputs: none
 * side effects: prints to video memory
 * function: prints all file names, ftype, and size
 */
void print_all_files(){
    uint32_t i,j;
    //clear the screen
    clear();
    resetCursor();
    for(i = 0; i < boot_block->num_dir_entries; i++){
        printf("File name: ");
        dentry_t d;
        //fill in dentry with directory info
        read_dentry_by_index(i,&d);
        for(j = 0; j < FNAME_LEN; j++)
            printf("%c",d.fname[j]);
        printf("File type: ");
        printf("%d",d.ftype);
        printf(" File size: %d\n",inodes[d.inode_num].len);
    }
}

/* void read_file_by_name
 * inputs: int8_t* name - file name to be read
 * outputs: none
 * side effects: prints to video memory
 * function: prints file contents to screen
 */
void read_file_by_name(int8_t* name){
    dentry_t d;
    uint32_t i,len;
    uint8_t buf[BUFLEN];
    //clear screen
    clear();
    resetCursor();
    //if file isnt found
    if(read_dentry_by_name((uint8_t*)name,&d) == -1){
        printf("File not found");
        return;
    }
    len = inodes[d.inode_num].len;
    //fill in buffer with file info
    fread(d.inode_num,0,(int8_t*)buf,len);
    //write to video memory
    terminal_write((int8_t*)buf,len);
    //print filename
    printf("\n");
    printf("File name: ");
    for(i = 0; i < FNAME_LEN; i++)
        printf("%c",d.fname[i]);
}

/* void read_file_by_index
 * inputs: none
 * outputs: none
 * side effects: prints to video memory
 * function: prints file contents to screen of all files
 */
void read_file_by_index(){
    dentry_t d;
    uint32_t i,len,j;
    int32_t key;
    uint8_t buf[BUFLEN];

    //iterate through all files
    for(i = 0; i < boot_block->num_dir_entries; i++){
        //used for press key to advance
        key = get_buf_idx();
        //get dentry_t for index i
        read_dentry_by_index(i,&d);
        //clear screen
        clear();
        resetCursor();
        len = inodes[d.inode_num].len;
        //read in information
        fread(d.inode_num,0,(int8_t*)buf,len);
        //display to screen
        terminal_write((int8_t*)buf,len);
        //print filename
        printf("\n");
        printf("File name: ");
        for(j = 0; j < FNAME_LEN; j++)
            printf("%c",d.fname[j]);
        //wait for key to be pressed
        while(key == get_buf_idx());
        bksp_handler();
    }
    clear();
}

/* void fopen
 * inputs: const int8_t* fname - file name
 * outputs: 0 for success, -1 for error
 * side effects: none
 * function: file open function
 */
int32_t fopen(const int8_t* fname){

    return 0;
}
/* void fread
 * inputs: uint32_t inode - inode number
            int8_t* buf - buffer to be filled
            int32_t nbytes - number of bytes to read
 * outputs: number of bytes read for success, -1 for error
 * side effects: fills buf
 * function: file read function
 */
int32_t fread(uint32_t inode, uint32_t offset, int8_t* buf, uint32_t nbytes){
    int32_t retval = read_data(inode,offset,(uint8_t*)buf,nbytes);
    return retval;
}
/* void fwrite
 * inputs: uint32_t inode - inode number
            int8_t* buf - buffer to be written from
            int32_t nbytes - number of bytes to write
 * outputs: number of bytes written for success, -1 for error
 * side effects: writes to file
 * function: file write function
 */
int32_t fwrite(uint32_t inode, uint32_t offset, const int8_t* buf, uint32_t nbytes){

    return -1;
}
/* void fclose
 * inputs: none
 * outputs: 0 for success, -1 for failure
 * side effects: none
 * function: file close function
 */
int32_t fclose(){
    return 0;
}

/* void dopen
 * inputs: none
 * outputs: 0 for success, -1 for failure
 * side effects: none
 * function: directory open function
 */
int32_t dopen(){

    return 0;
}
/* void dread
 * inputs: const int8_t* fname - name of directory
            int8_t* buf - buffer to read in to
            int32_t nbytes - how many bytes to read
 * outputs: 0 for success, -1 for failure
 * side effects: none
 * function: directory read function
 */
int32_t dread(const int8_t* fname, dentry_t* buf){
    return read_dentry_by_name((const uint8_t*)fname,buf);
}


/* void dread_idx
 * inputs:  int32_t idx - index of directory
            int8_t* buf - buffer to read in to
 * outputs: FNAME_LEN for success, -1 for failure
 * side effects: none
 * function: reads in filename into buffer of directory
 */
int32_t dread_idx(int32_t idx, int8_t* buf){
    if(idx >= boot_block->num_dir_entries)
        return 0;
    int32_t len = 0;
    dentry_t d;
    read_dentry_by_index(idx,&d);
    strncpy(buf,d.fname,FNAME_LEN);
    while(len < FNAME_LEN && buf[len] != '\0') len++;

    return len;
}
/* void dwrite
 * inputs: const int8_t* fname - name of directory
            int8_t* buf - buffer to write from
            int32_t nbytes - how many bytes to write
 * outputs: 0 for success, -1 for failure
 * side effects: none
 * function: directory write function
 */
int32_t dwrite(const const int8_t* buf){

    return -1;
}
/* dclose
 * inputs:none
 * outputs: 0 for success, -1 for failure
 * side effects: none
 * function: directory close function
 */
int32_t dclose(){

    return 0;
}


/* get_length
 * inputs:inode number
 * outputs: length of file in bytes
 * side effects: none
 * function: used to get length of file
 */
uint32_t get_length(uint32_t inode){
    return inodes[inode].len;
}

/* get_idx
 * inputs:inode number
 * outputs: index of dentry in fs
 * side effects: none
 * function: used to get index of a file given its inode
 */
int32_t get_idx(uint32_t inode){
    int32_t i;
    for(i = 0; i < MAX_DENTRY; i++){
        if(dentries[i].inode_num == inode)
            return i;
    }
    return -1;
}


/* f_driver
 * input: uint32_t cmd - command number
 *        uint32_t fd - file descriptor
 *        void* buf - buffer to read/write
 *        uint32_t byte_count - how many bytes to read/write
 * output: return values from respective o/c/r/w functions, -1 if invalid cmd
 * side effects:
 * function: dispatcher function
 */
int32_t f_driver(uint32_t cmd, uint32_t fd, void* buf, uint32_t nbytes){
    //open
    if(cmd == OPEN){
        return fopen((const int8_t*)buf);
    }
    //read
    else if(cmd == READ){
        uint32_t inode = curr_pcb->file_arr[fd].inode;
        uint32_t offset = curr_pcb->file_arr[fd].position;
        int32_t bytes_read = fread(inode,offset,(int8_t*)buf,nbytes);
        if(bytes_read != -1)
          curr_pcb->file_arr[fd].position += bytes_read;
        return bytes_read;
    }
    //write
    else if(cmd == WRITE){
        //need to find access pcb
        uint32_t inode = curr_pcb->file_arr[fd].inode;
        uint32_t offset = curr_pcb->file_arr[fd].position;
        int32_t bytes_written = fread(inode,offset,(int8_t*)buf,nbytes);
        if(bytes_written != -1)
          curr_pcb->file_arr[fd].position += bytes_written;
        return bytes_written;
    }
    //close
    else if(cmd == CLOSE){
        return fclose();
    }
    return -1;
}

/* d_driver
 * input: uint32_t cmd - command number
 *        uint32_t fd - file descriptor
 *        void* buf - buffer to read/write
 *        uint32_t byte_count - how many bytes to read/write
 * output: return values from respective o/c/r/w functions, -1 if invalid cmd
 * side effects:
 * function: dispatcher function
 */
int32_t d_driver(uint32_t cmd, uint32_t fd, void* buf, uint32_t nbytes){
    if(cmd == OPEN){
        return dopen();
    }
    else if(cmd == READ){
        uint32_t idx = curr_pcb->file_arr[fd].position;
        curr_pcb->file_arr[fd].position++;
        return dread_idx(idx,(int8_t*)buf);
    }
    else if(cmd == WRITE){
        return dwrite((const int8_t*)buf);
    }
    else if(cmd == CLOSE){
        return dclose();
    }
    return -1;
}
