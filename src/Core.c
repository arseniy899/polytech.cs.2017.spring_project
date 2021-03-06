#include "LibraryMerger.h"

ALLEGRO_FONT* font = NULL, *AllegroFont = NULL; //!< Allegro fonts for storing fonts
int SCREEN_WIDTH = 0, SCREEN_HEIGHT = 0, SCENE_NOW = 0, EventManagerThreadRunning = 1;

ALLEGRO_DISPLAY* display = NULL;//!< Allegro's display for storing display
float SCREEN_WIDTH_UNIT = 0, SCREEN_HEIGHT_UNIT = 0;
ALLEGRO_PATH *path;
FILE *targetFile;
char * resourcePath;
ALLEGRO_SAMPLE *menuSample = NULL, *levelSample = NULL;
int initAddons()
{
   al_init();
   printf("%0x, %0x",ALLEGRO_VERSION_INT,al_get_allegro_version());
   al_init_font_addon();
   if(!al_init_ttf_addon())
   {
      Log_e(__func__, "failed to initialize TTF!\n");
      return -2;
   }
   else if(!al_init_primitives_addon())
   {
      Log_e(__func__, "failed to initialize primitives!\n");
      return -3;
   }
   al_install_keyboard();
   if(!al_init_native_dialog_addon())
   {
      Log_e(__func__, "failed to initialize dialogs!\n");
      return -5;
   }
   else if(!al_init_image_addon())
   {
      Log_e(__func__, "failed to initialize images!\n");
      return -6;
   }
   else if (!al_install_audio()) 
   {
      Log_e(__func__, "failed to initialize audio!\n");
      return -7;
   }
   else if (!al_init_acodec_addon()) {
      Log_e(__func__, "failed to initialize audio codecs!\n");
      return -8;
   }
   else if (!al_reserve_samples(2)) {
      Log_e(__func__, "failed to reserve samples!\n");
      return -9;
   }
   else return 0;
}
int openFolderDialog(int i)
{

   ALLEGRO_FILECHOOSER *dialog = al_create_native_file_dialog("","Choose folder","*.*",ALLEGRO_FILECHOOSER_FOLDER);
   if (al_show_native_file_dialog(display, dialog))
   {
       convertConstCopy(al_get_native_file_dialog_path(dialog, 0), &pathCur);
       showDirectoryListing(0);
   }
   return 0;
}
int openLevelSelect(int i)
{
   clearButtons(LEVEL_SELECT_SCENE);
   clearSpritesScene(LEVEL_SELECT_SCENE);
   changeScene(LEVEL_SELECT_SCENE);
   showDirectoryListing(0);
   renderScreen();
   return 0;
}

int sliceFile(int i)
{
   
   char * pathSource = NULL, *pathTarget = NULL;
   ALLEGRO_FILECHOOSER *dialog = al_create_native_file_dialog("", CH_SOURCE, "*.txt", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
   if (al_show_native_file_dialog(display, dialog))
   {
      int c = 0;  
      convertConstCopy(al_get_native_file_dialog_path(dialog, 0), &pathSource);
      al_destroy_native_file_dialog(dialog);
      dialog = al_create_native_file_dialog("", CH_FOLDER, "*.*", ALLEGRO_FILECHOOSER_FOLDER);
      if (al_show_native_file_dialog(display, dialog))
      {
         convertConstCopy(al_get_native_file_dialog_path(dialog, 0), &pathTarget);
         char destFileStr[DEFAULT_LENGTH_STR] = { 0 };
         FILE *sourceFile = fopen(pathSource, "r");
         sprintf(destFileStr, "%s/%03i.txt", pathTarget, ++c);
         FILE *destFile = fopen(destFileStr, "w+");
         if (sourceFile == NULL)
         {
            Log_e(__func__, "Source file error!\n");
            if(destFile)
               fclose(destFile);
            if (pathTarget)
               free(pathTarget);
            if (sourceFile)
               fclose(sourceFile);
            return -1;
         }
         char str[DEFAULT_LENGTH_STR];
         bool prevStrIsData = false;
         while (fgets(str, sizeof(str), sourceFile) != NULL)
         {

            Log_i(__func__, "destFileStr=%s,str=%s", destFileStr,str);
            int occur = 0;
            if (strstr(str,"#"))
               occur+=2;
            if (strstr(str,"."))
               occur++;
            if (strstr(str,"@"))
               occur+=2;
            if (strstr(str,"$"))
               occur+=2;
            if (strstr(str," "))
               occur++;
            if (occur <= 1)
            {
               //if (prevStrIsData)
                  //fprintf(destFile, "%s", str);
               prevStrIsData = false;
               
            }
            else
            {
               
               if (!prevStrIsData)
               {
                  sprintf(destFileStr, "%s/%3i.txt", pathTarget, c++);
                  fclose(destFile);
                  destFile = fopen(destFileStr, "w+");
                  fprintf(destFile, "%s", str);
               }
               else fprintf(destFile, "%s", str);
               if (destFile == NULL)
               {
                  Log_e(__func__, "destFile file error!\n");
                  return -1;
               }
               
               prevStrIsData = true;
            }
         }
         fclose(destFile);
         fclose(sourceFile);
         al_show_native_message_box(display, WH_COMPL, SLICE_SUCC, "", NULL, 0);
         al_destroy_native_file_dialog(dialog);
      }
      
   }
   if(pathSource)
      free(pathSource);
   return 0;
}
int nextPage(int i)
{
   levelSelectPage++;
   clearButtons(LEVEL_SELECT_SCENE);
   clearSpritesScene(LEVEL_SELECT_SCENE);
   showDirectoryListing(0);
   renderScreen();
   return 0;
}
int prevPage(int i)
{
   if (levelSelectPage == 0)
      return 1;
   levelSelectPage--;

   clearButtons(LEVEL_SELECT_SCENE);
   clearSpritesScene(LEVEL_SELECT_SCENE);
   showDirectoryListing(0);
   renderScreen();
   return 0;
}
int count = 0;
int showDirectoryListing(int i)
{
   clearButtons(LEVEL_SELECT_SCENE);
   clearSpritesScene(LEVEL_SELECT_SCENE);
   addButtonSprite(DEF_BUT_SPR, CH_FOLDER, SCREEN_WIDTH_UNIT * 1600, SCREEN_WIDTH_UNIT * 1, SCREEN_WIDTH_UNIT * 400, SCREEN_WIDTH_UNIT * 100, 255, 255, 255, openFolderDialog, LEVEL_SELECT_SCENE, 1, 4);
   
   addSprite(SPR_WIN_BACK, SCREEN_WIDTH_UNIT * 350, SCREEN_HEIGHT_UNIT * 100, SCREEN_WIDTH_UNIT * 1300, SCREEN_HEIGHT_UNIT * 1700, SCENE_NOW, 1);
   addSprite(ACT_BACK_SPR, SCREEN_WIDTH_UNIT * 750, SCREEN_HEIGHT_UNIT * 1700, 500 * SCREEN_WIDTH_UNIT, SCREEN_HEIGHT_UNIT * 200, SCENE_NOW, 1);
   addSprite(BACK_TITLE_SPR, SCREEN_WIDTH_UNIT * 550, SCREEN_HEIGHT_UNIT * 50, 950 * SCREEN_WIDTH_UNIT, SCREEN_HEIGHT_UNIT * 200, SCENE_NOW, 1);
   addLabel(SCREEN_WIDTH_UNIT * 1025, SCREEN_HEIGHT_UNIT * 90, 255, 90, 0, SCENE_NOW, ALLEGRO_ALIGN_CENTER, LEV_SEL_LABEL);


   addButtonSprite(DEF_BUT_SPR, NEXT_SYMB, SCREEN_WIDTH_UNIT * 1100, SCREEN_HEIGHT_UNIT * 1700, SCREEN_WIDTH_UNIT * 100, SCREEN_WIDTH_UNIT * 100, 255, 255, 255, nextPage, LEVEL_SELECT_SCENE, 1, 4);
   addButtonSprite(DEF_BUT_SPR, PREV_SYMB, SCREEN_WIDTH_UNIT * 800, SCREEN_HEIGHT_UNIT * 1700, SCREEN_WIDTH_UNIT * 100, SCREEN_WIDTH_UNIT * 100, 255, 255, 255, prevPage, LEVEL_SELECT_SCENE, 1, 4);
   addButtonSprite(REPL_SPR, "", SCREEN_WIDTH_UNIT * 900, SCREEN_HEIGHT_UNIT * 1700, SCREEN_WIDTH_UNIT * 100, SCREEN_WIDTH_UNIT * 100, 255, 255, 255, showDirectoryListing, LEVEL_SELECT_SCENE, 1, 4);
   addButtonSprite(OPEN_SPR, "", SCREEN_WIDTH_UNIT * 1000, SCREEN_HEIGHT_UNIT * 1700, SCREEN_WIDTH_UNIT * 100, SCREEN_WIDTH_UNIT * 100, 255, 255, 255, openFolderDialog, LEVEL_SELECT_SCENE, 1, 4);
   
   ALLEGRO_FS_ENTRY* dir = al_create_fs_entry(pathCur);

   while (count > 0)
      sprintf(levelsNames[count--], "%s","");
   renderScreen();
   if (al_open_directory(dir))
   {
      ALLEGRO_FS_ENTRY* file;
      Log_i(__func__, "LISTING");
      int(*callBacks[64])(int id);
        memset(levelsPaths,0,sizeof(levelsPaths));
      while ((file = al_read_directory(dir)))
      {
         if (file == NULL)
            Log_e(__func__, "ERROR: Level file is null");
         else
         {
            //printf("path:%s\n", al_get_fs_entry_name(file));
            char *path = NULL;
            convertConstCopy(al_get_fs_entry_name(file), &path);
            char *filename;
                filename = strrchr(path, '\\');
            if (filename == NULL)
            {
               filename = strrchr(path, '/');
               if (filename == NULL)
                  filename = path;
               else
                  filename++;
            }
            else
               filename++;
            if (strstr(filename, ".txt"))
            {
               
               if (count >= levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE && (count) < (levelSelectPage + 1) * LEVEL_SELECT_ITEMS_PER_PAGE)
               {
                  sprintf(levelsPaths[count - levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE], "%s", path);
                  filename[strlen(filename) - 4] = 0;
                  sprintf(levelsNames[count- levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE], "%s", filename);
                  callBacks[count- levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE] = &openLevel;
                  Log_i(__func__, "Level: %s", filename);


               }
               count++;
            }
            if (count > (levelSelectPage + 1) * LEVEL_SELECT_ITEMS_PER_PAGE)
               break;
         }
      }
      al_destroy_fs_entry(file);
      if ((count - levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE) > LEVEL_SELECT_ITEMS_PER_PAGE)
         count--;
      if ((count - levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE)  <= 0)
         prevPage(0);
      makeGridSprites(count- levelSelectPage * LEVEL_SELECT_ITEMS_PER_PAGE, 4, levelsNames, DEF_BUT_SPR, SCREEN_WIDTH_UNIT * 650, SCREEN_HEIGHT_UNIT * 400, SCREEN_WIDTH_UNIT * 150, SCREEN_WIDTH_UNIT * 150, callBacks, LEVEL_SELECT_SCENE, 1);
   }
   al_destroy_fs_entry(dir);
   Log_i(__func__, "Folder: %s", pathCur);
   return 0;
}
int ExitProg(int i)
{
   EventManagerThreadRunning = 0;
   return 0;
}
int openLevelEditor(int i)
{
   changeScene(LEVEL_EDITOR_SCENE);
   onLevelEditorOpened();
   return 0;
}
void renderScreen()
{
   if(!display)
      return;
   al_clear_to_color(al_map_rgb(0, 0, 0));
   for (int i = 0; i < SPRITES_LAYERS_AMOUNT; i++)
      renderSprites(i);
   renderSprites(-1);
   renderButtonsSc();
   renderLabelsSc();
   al_flip_display();

   //al_create_thread(renderSpriteThr, NULL, -1);

}
void changeScene(int scene)
{
   if (SCENE_NOW != scene)
   {
      switch (scene)
      {
         case MAINMENU_SCENE:
            al_stop_samples();
            al_play_sample(menuSample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
         break;
         case LEVEL_SCENE:
            al_stop_samples();
            al_play_sample(levelSample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
         break;
         case LEVEL_SCENE_PAUSE:
         case LEVEL_EDITOR_SCENE:
            al_stop_samples();
         break;
      }
   }
   SCENE_NOW = scene;
   renderScreen();

}
void Log_i(const char * tag, const char *str, ...)
{
   char buf[DEFAULT_LENGTH_STR];
   va_list vl;
   va_start(vl, str);
   vsnprintf(buf, sizeof(buf), str, vl);
   va_end(vl);
   printf("I/%s::%s\n", tag, buf);
   if(targetFile != NULL)
   {
      fprintf(targetFile, "I/%s::%s\n",tag, buf);
      fflush(targetFile);
   }
}
void Log_e(const char * tag, const char *str, ...)
{
   char buf[DEFAULT_LENGTH_STR];
   va_list vl;
   va_start(vl, str);
   vsnprintf(buf, sizeof(buf), str, vl);
   va_end(vl);
   printf("E/%s::%s\n", tag, buf);
   if(targetFile != NULL)
   {
      fprintf(targetFile, "E/%s::%s\n", tag, buf);
      fflush(targetFile);
   }
   if(display)
      al_show_native_message_box(display,"Error!","An ERROR accuredued:",buf,NULL, ALLEGRO_MESSAGEBOX_ERROR);
}
void setNewScreen()
{


   ALLEGRO_DISPLAY_MODE  disp_data;
   al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);
   al_acknowledge_resize(display);
   float newWidth = al_get_display_width(display), newHeight = al_get_display_height(display);
   float wC = newWidth/SCREEN_WIDTH, hC = newHeight / SCREEN_HEIGHT ;
   SCREEN_WIDTH = newWidth; SCREEN_HEIGHT = newHeight;
   al_resize_display(display, SCREEN_WIDTH, SCREEN_HEIGHT);
   Log_i(__func__, "New screen:%i %i, coef=%f,%f", SCREEN_WIDTH, SCREEN_HEIGHT, wC,hC);
   al_set_new_display_flags(ALLEGRO_FULLSCREEN);
   SCREEN_WIDTH_UNIT = SCREEN_WIDTH / 2000.0;
   SCREEN_HEIGHT_UNIT = SCREEN_HEIGHT / 2000.0;
   //SCREEN_WIDTH -= 250; SCREEN_HEIGHT -= 40;
   recalcSprites(wC, hC);
   recalcButtons(wC, hC);
   recalcLabels(wC, hC);
   font = al_load_ttf_font("SansPosterBold.ttf", SCREEN_HEIGHT_UNIT * 60, 0);
   renderScreen();
   Log_i(__func__, "New screen set");
}
int initVars()
{
   srand(time(NULL));
   AllegroFont = al_create_builtin_font();
   printf("font created\n");
   targetFile = fopen(LOGS_PATH, "a+");
   if (targetFile == NULL)
   {
      printf("Target file error!\n");
      return -1;
   }
   printf("file opened\n");
   ALLEGRO_DISPLAY_MODE  disp_data;
   al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);

   SCREEN_WIDTH = disp_data.width; SCREEN_HEIGHT = disp_data.height;
   
   Log_i(__func__,"Screen:%i %i", SCREEN_WIDTH, SCREEN_HEIGHT);
   
   /*if(al_show_native_message_box(display, "FullScreen", "Would you like to run in fullscreen mode?", "", NULL, ALLEGRO_MESSAGEBOX_YES_NO))
      al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
   else
   {
      if (DEBUG_MODE)
      {*/
         SCREEN_WIDTH /= 2; SCREEN_HEIGHT /= 2;
      /*}
      else
         SCREEN_HEIGHT -= SCREEN_HEIGHT_UNIT*200;*/
      al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
      
   //}
   path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);

   al_append_path_component(path, RES_FIRS);
   Log_i(__func__, "Res path:%s", al_path_cstr(path, '/'));
   if (!al_change_directory(al_path_cstr(path, '/')))
   {
      path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
      al_append_path_component(path, RES_SEC);
      if (!al_change_directory(al_path_cstr(path, '/')))
      {
         //const char * p = al_path_cstr(path, '/');
         //Log_e(__func__, "Res path not found: %s", p);
         path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
         al_append_path_component(path, RES_THIR);
          if (!al_change_directory(al_path_cstr(path, '/')))
         {
              Log_e(__func__, "Res path not found: %s", al_path_cstr(path, '/'));
              return -2;
          }
      }
   }
   
   const char *p = al_path_cstr(path, '/');
   resourcePath = (char*)malloc(strlen(p)+1);
   sprintf(resourcePath ,"%s",p);
   SCREEN_WIDTH_UNIT = SCREEN_WIDTH / 2000.0;
   SCREEN_HEIGHT_UNIT = SCREEN_HEIGHT / 2000.0;
   font = al_load_ttf_font(TTF_SRC_LINK, -SCREEN_HEIGHT_UNIT * 80, ALLEGRO_TTF_MONOCHROME);
   display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
   ALLEGRO_BITMAP * icon = al_load_bitmap(LOGO_SPR_LINK);
   if (icon == NULL)
   {
      Log_e(__func__, "ERROR Loading sprite %s:No file", LOGO_SPR_LINK);
      return -1;
   }
   al_set_display_icon(display, icon);
   menuSample = al_load_sample(MENU_SAMPLE);

   if (!menuSample) {
      Log_e(__func__, "Audio clip menuSample not loaded!\n");
      return -1;
   }
   levelSample = al_load_sample(LEVEL_SAMPLE);

   if (!levelSample) {
      Log_e(__func__, "Audio clip levelSample not loaded!\n");
      return -1;
   }
   al_play_sample(menuSample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
   loadMem();
   //SCREEN_WIDTH -= 250; SCREEN_HEIGHT -= 40;
   //al_clear_to_color(al_map_rgb(0, 0, 0));
   return 0;

}
int openLevel(int num)
{

	printf("\nOpening Level #%s", levelsPaths[num]);
	FILE *sourceFile = fopen(levelsPaths[num], "r");
	if (!sourceFile)
	{
		Log_e(__func__, "ERROR: Source file for level not found: %s", levelsPaths[num]);
		return -1;
	}
	char str[DEFAULT_LENGTH_STR];
	int y = 0;
	for (int q = 0; q < LEVEL_HEIGHT; q++)
	{
		for (int i = 0; i < LEVEL_WIDTH; i++)
			map[q][i] = 0;
	}
	int playerX = 0, playerY = 0, cratesCount = 0; bool playerExists = false;
	while (fgets(str, sizeof(str), sourceFile) != NULL)
	{
		for (int i = 0; i < strlen(str); i++)
		{
			switch (str[i])
			{
				case '1':
				case '#':
					map[i][y] = 1;
				break;
				case '3':
				case '$':
					cratesCount++;
					map[i][y] = 3;
				break;
				case '2':
				case '.':
					map[i][y] = 2;
				break;
				case '9':
				case '@':
				case '+':
					playerX = i; playerY=y;
					playerExists = true;
				break;
				case '0':
				case ' ':
					map[i][y] = 0;
				break;
				case '4':
				case '*':
					map[i][y] = 4;
				break;
				default:
					if (str[i] != '\0' && str[i] != '\n')
				       	{
				  		Log_e(__func__, "input file was in incorrect format (symbols only ' ','@','#','.','*','$','+')");
						return -1;
				       	}
				break;
			}
		}
		
		y++;
		Log_i(__func__, "levelStr=%s", str);


	}
	if (!playerExists)
	{
		Log_e(__func__, "input file has no position for player! Aborting...");
		return -2;
   }
	fclose(sourceFile);
	changeScene(LEVEL_SCENE);
	//onLevelOpened(num);
	onLevelFileOpened(num, playerX, playerY, cratesCount);
	return 0;
}
void convertConstCopy(const char *source, char **toChr)
{
    //if(*toChr && *toChr && *toChr != NULL && sizeof(*toChr))
       // free(*toChr);
    *toChr = (char*)malloc(strlen(source)+1);
    sprintf(*toChr ,"%s",source);
}
