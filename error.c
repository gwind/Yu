#include "yu.h"
#include "error.h"

/* 本文件定义常见错误，并处理之 */


int yu_err (int err)
{
  switch (err) 
    {

    case YU_OPT_ERR:
      printf (_("Your options have a error! Please check again.\n"));
      break;

    case YU_CONFIG_ERR:
      printf (_("Read config file have a error!\n"));
      break;

    default:
      printf (_("Unknown error resson!\n"));
      break;
    }
  return err;
}
