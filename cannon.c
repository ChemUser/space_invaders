#include "primlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_OF_BULLETS 3

struct Bullet
   {
	   double x;
	   double y;
	   int ex_radius;
	   int hit;
	   int shot;
	   int period;
   };

enum trajectory { linear , sine , rectangular };

struct Target
	{
		struct position
		{
			double x;
			double y;
			double offset;
		} pos;
		double speed;
		int waiting;
		enum trajectory path;
	};

void drawCannon( double cannonX );

void drawBul_Tar( struct Bullet *bul , struct Target *tar , struct Bullet *enemy_bul , int num );

void moveTarget( struct Target *tar , int num );

void hit( struct Bullet *bul , struct Target *tar , struct Bullet *enemy_bul , double hit_dist , double cannonX , int num );

void newLevel( struct Target *tar , struct Bullet *enemy_bul , struct Bullet *bul , int num );

void score();

int shot_num = 0;
int kil_num = 0;
int lives = 4;

char str[12];

int main() {
  if (gfx_init())
    exit(3);

  srand( time( NULL ) );

  int levels[] = { 5 , 13 , 25 , 38 };
  int level = 0;

  struct Bullet *bullet = ( struct Bullet *) malloc( NUM_OF_BULLETS * sizeof( struct Bullet ) );
  struct Target *target = ( struct Target *) malloc( levels[ level ] * sizeof( struct Target ) );
  struct Bullet *enemy_bul = ( struct Bullet *) malloc( levels[ level ] * sizeof( struct Bullet ) );

  for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )//bullet initialisation
  {
	  bullet[i].shot = 0;
	  bullet[i].hit = 0;
	  bullet[i].ex_radius = 0;
  }

  for( int i = 0 ; i < levels[ level ] ; i++ )
  {
		target[i].waiting = 1;

		enemy_bul[i].shot = 0;
	  	enemy_bul[i].hit = 0;
	  	enemy_bul[i].ex_radius = 0;
		enemy_bul[i].period = rand() % 250 + 251;
  }

  double cannonX = gfx_screenWidth() / 2.0;//cannon position

  double hit_dist = 20.0;

  while( 1 )
  {
	
	for ( int i = 0; i < levels[ level ]; i++ )// target initialisation ; rand()%150 makes targets appear in random time stamps , but not too seldom
	{
		if( rand() % 150 == 0 && target[i].waiting )
		{
			target[i].waiting = 0;
			target[i].speed = rand() % 3 + 1.5;
			target[i].pos.x = 0;
			target[i].pos.y = 85.0 * ( i + 1 );
			target[i].pos.offset = 85.0 * ( i + 1 );
			target[i].path = rand() % 3;
		}
	}

	gfx_filledRect( 0 , gfx_screenHeight() - 1 , gfx_screenWidth() - 1 , 0 , BLUE );//drawing screen background

    drawCannon( cannonX );

    drawBul_Tar( bullet , target , enemy_bul , levels[ level ] );

	hit( bullet , target , enemy_bul , hit_dist , cannonX , levels[ level ] );

	score();

	for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )
    {
		if( bullet[i].shot )
		{
			bullet[i].y -= 5.0;
		}

		if( bullet[i].hit )//animation of explosion
		{
			if( bullet[i].ex_radius == 50 )
			{
				bullet[i].shot = 0;
				bullet[i].hit = 0;
				bullet[i].ex_radius = 0;
			}
			else 
			{
				for( int k = bullet[i].ex_radius/10 ; k > 0 ; k-- )//animation is an expanding circle consisting of yellow and red rings
				{
					( k % 2 == 0 ) ? gfx_filledCircle( bullet[i].x , bullet[i].y , k*10 , RED ) : gfx_filledCircle( bullet[i].x , bullet[i].y , k*10 , YELLOW );
				}
				bullet[i].ex_radius += 10;
			}
		}
    }

	for( int i = 0 ; i < levels[ level ] ; i++ )
	{
		if( !enemy_bul[i].shot && !target[i].waiting )
		{
			enemy_bul[i].period--;
			if( enemy_bul[i].period == 0 )
			{
				enemy_bul[i].shot = 1;
				enemy_bul[i].x = target[i].pos.x;
				enemy_bul[i].y = target[i].pos.y;
			}
		}	
	}

	for( int i = 0 ; i < levels[ level ] ; i++ )
    {
		if( enemy_bul[i].shot )
		{
			enemy_bul[i].y += 5.0;
		}

		if( enemy_bul[i].hit )//animation of explosion
		{
			if( enemy_bul[i].ex_radius == 50 )
			{
				enemy_bul[i].shot = 0;
				enemy_bul[i].hit = 0;
				enemy_bul[i].ex_radius = 0;
				enemy_bul[i].period = rand() % 250 + 251;
			}
			else 
			{
				for( int k = enemy_bul[i].ex_radius/10 ; k > 0 ; k-- )//animation is an expanding circle consisting of yellow and red rings
				{
					( k % 2 == 0 ) ? gfx_filledCircle( enemy_bul[i].x , enemy_bul[i].y , k*10 , RED ) : gfx_filledCircle( enemy_bul[i].x , enemy_bul[i].y , k*10 , YELLOW );
				}
				enemy_bul[i].ex_radius += 10;
			}
		}
    }

    gfx_updateScreen();

    moveTarget( target , levels[ level ] );

    if( gfx_isKeyDown(SDLK_RIGHT) && cannonX < gfx_screenWidth() - 100.0 )
     cannonX += 3.5;
    if( gfx_isKeyDown(SDLK_LEFT) && cannonX > 100.0 )
     cannonX -= 3.5;
    if( gfx_isKeyDown(SDLK_SPACE) )// bullet is fired if distance of previously shot bullet is more than 275 pixels from the cannon or if no bullet is shot
    {
		double min_dist = gfx_screenWidth() * 2;//here we choose a value which is impossible, for later comparison
		for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )
		{
			if( bullet[i].shot )
			{
				if( min_dist > ( gfx_screenHeight() - bullet[i].y ) )
				{
					min_dist = gfx_screenHeight() - bullet[i].y;
				}
			}
		}
		for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )
		{
			if( !bullet[i].shot && ( min_dist > 275.0 || min_dist > gfx_screenWidth() ) )
			{
				bullet[i].shot = 1;
				bullet[i].y = gfx_screenHeight() - 170.0;
				bullet[i].x = cannonX;
				shot_num++;
				goto exit;
			}
        }

    }

	if( kil_num == levels[ level ] )
	{
		lives++;
		level++;

		free( enemy_bul );
		free( target );

		printf("target - %p",target);

		enemy_bul = ( struct Bullet *) malloc( levels[ level ] * sizeof( struct Bullet ) );
		target = ( struct Target *) malloc( levels[ level ] * sizeof( struct Target ) );

		newLevel( target , enemy_bul , bullet , levels[ level ] );

		printf("target - %p",target);
	}

	exit:
	{
		;
	}

	if( level == 4 )
	{
		gfx_filledRect( 0 , gfx_screenHeight() - 1 , gfx_screenWidth() - 1 , 0 , BLACK );
		gfx_textout( gfx_screenWidth() / 2 , gfx_screenHeight() / 2 - 15 , "CONGRATULATIONS ! YOU BEAT THE GAME. " , WHITE );
		sprintf( str , "%d" , kil_num );
		char source[] = "Number of points:";
		gfx_textout( gfx_screenWidth() / 2 , gfx_screenHeight() / 2 , strcat( source , str ) , WHITE );
		gfx_updateScreen();

		free( bullet );
		free( target );
		free( enemy_bul );

		//black screen
		while(1)
		{
			if(gfx_isKeyDown(SDLK_ESCAPE))
    		{
      			exit(4);
    		}
		}
	}

	//game over
	if( lives == 0 )
	{
		gfx_filledRect( 0 , gfx_screenHeight() - 1 , gfx_screenWidth() - 1 , 0 , BLACK );
		gfx_textout( gfx_screenWidth() / 2 , gfx_screenHeight() / 2 - 15 , "GAME OVER:" , WHITE );
		sprintf( str , "%d" , kil_num );
		char source[] = "Number of points:";
		gfx_textout( gfx_screenWidth() / 2 , gfx_screenHeight() / 2 , strcat( source , str ) , WHITE );
		gfx_updateScreen();

		free( bullet );
		free( target );
		free( enemy_bul );

		//black screen
		while(1)
		{
			if(gfx_isKeyDown(SDLK_ESCAPE))
    		{
      			exit(4);
    		}
		}
	}

	if(gfx_isKeyDown(SDLK_ESCAPE))
    {
		free( bullet );
		free( target );
		free( enemy_bul );

    	exit(4);
    }
    SDL_Delay(12);
  }

  return 0;

}

void moveTarget( struct Target *tar , int num )
{
	for( int i = 0 ; i < num ; i++ )
	{
		switch( tar->path )
		{
			case 0:
			{
				tar->pos.x += tar->speed;
			}
			break;

			case 1:
			{
				tar->pos.x += tar->speed;
				tar->pos.y = tar->pos.offset + 22.5 * sin( 6 * tar->pos.x * M_PI / gfx_screenWidth() );
			}
			break;
			
			case 2:
			{
				tar->pos.x += tar->speed;
				tar->pos.y = tar->pos.offset + 22.5 * ( 2.0 * ( 2.0 * floor( 6.0 / gfx_screenWidth() * tar->pos.x ) - floor( 12.0 / gfx_screenWidth() * tar->pos.x ) ) + 1.0 );			
			}
			break;

		}
		if( tar->pos.x > gfx_screenWidth() )//when target goes of the screen it is respawned at the start
		{
			tar->pos.x = 0.0;
		}
		tar++;
	}
}

void hit( struct Bullet *bul , struct Target *tar , struct Bullet *enemy_bul , double hit_dist , double cannonX , int num )
{
	struct Target *save = tar;

	for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )
	{
		tar = save;

		for( int j = 0 ; j < num ; j++ )
		{
			if( sqrt( pow( bul->x - tar->pos.x , 2 ) + pow( bul->y - tar->pos.y , 2 ) ) < hit_dist && bul->shot && !tar->waiting )
			{
				bul->hit = 1;
				tar->waiting = 1;
				kil_num++;
			}
			tar++;
		}

		bul++;
	}
	
	for( int i = 0 ; i < num ; i++ )
	{
		if( enemy_bul->x > cannonX - 80.0 && enemy_bul->x < cannonX + 80.0 && enemy_bul->shot && !enemy_bul->hit )
		{
			if( enemy_bul->y > gfx_screenHeight() - 150.0 || ( enemy_bul->x > cannonX - 20.0 && enemy_bul->x < cannonX + 20.0 && enemy_bul->y > gfx_screenHeight() - 170.0 ) )
			{
				lives--;
				enemy_bul->hit = 1;
			}
		}
		enemy_bul++;
	}
} 


void drawCannon( double cannonX )
{
	gfx_filledRect( cannonX - 20 , gfx_screenHeight() , cannonX + 20 , gfx_screenHeight() - 170 , BLACK );
	gfx_filledCircle( cannonX , gfx_screenHeight() - 170 , 10 , BLUE );
	gfx_filledTriangle( cannonX + 20 , gfx_screenHeight() , cannonX + 20 , gfx_screenHeight() - 125 , cannonX + 70.7 , gfx_screenHeight() - 30.1 , WHITE );
	gfx_filledTriangle( cannonX - 20 , gfx_screenHeight() , cannonX - 20 , gfx_screenHeight() - 125 , cannonX - 70.7 , gfx_screenHeight() - 30.1 , WHITE );
	gfx_filledTriangle( cannonX + 20 , gfx_screenHeight() - 125 , cannonX + 70.7 , gfx_screenHeight() - 30.1 , cannonX + 80 , gfx_screenHeight() - 125 , RED );
	gfx_filledTriangle( cannonX - 20 , gfx_screenHeight() - 125 , cannonX - 70.7 , gfx_screenHeight() - 30.1 , cannonX - 80 , gfx_screenHeight() - 125 , RED );
	gfx_filledTriangle( cannonX + 80 , gfx_screenHeight() - 125 , cannonX + 80 , gfx_screenHeight() - 150 , cannonX + 40 , gfx_screenHeight() - 125 , RED );
	gfx_filledTriangle( cannonX - 80 , gfx_screenHeight() - 125 , cannonX - 80 , gfx_screenHeight() - 150 , cannonX - 40 , gfx_screenHeight() - 125 , RED );
}


void drawBul_Tar( struct Bullet *bul , struct Target *tar , struct Bullet *enemy_bul , int num )
{

    for( int j = 0 ; j < num ; j++ )
	{
		if( !tar->waiting )
		{

			gfx_filledTriangle( tar->pos.x + 7.1 , tar->pos.y , tar->pos.x , tar->pos.y - 7.1 , tar->pos.x + 10.6 , tar->pos.y - 10.6 , YELLOW );
			gfx_filledTriangle( tar->pos.x - 7.1 , tar->pos.y , tar->pos.x , tar->pos.y - 7.1 , tar->pos.x - 10.6 , tar->pos.y - 10.6 , YELLOW );
			gfx_filledTriangle( tar->pos.x - 7.1 , tar->pos.y , tar->pos.x , tar->pos.y + 7.1 , tar->pos.x - 10.6 , tar->pos.y + 10.6 , YELLOW );
			gfx_filledTriangle( tar->pos.x , tar->pos.y + 7.1 , tar->pos.x + 7.1 , tar->pos.y , tar->pos.x + 10.6 , tar->pos.y + 10.6 , YELLOW );

			gfx_filledRect( tar->pos.x - 5 , tar->pos.y - 5 , tar->pos.x + 5 , tar->pos.y + 5 , BLACK );
			gfx_filledCircle( tar->pos.x , tar->pos.y , 5 , RED );

			gfx_filledTriangle( tar->pos.x - 5 , tar->pos.y + 5 , tar->pos.x + 5 , tar->pos.y + 5 , tar->pos.x , tar->pos.y + 15 , BLACK );
			gfx_filledTriangle( tar->pos.x - 5 , tar->pos.y - 5 , tar->pos.x + 5 , tar->pos.y - 5 , tar->pos.x , tar->pos.y - 15 , BLACK );
			gfx_filledTriangle( tar->pos.x + 5 , tar->pos.y + 5 , tar->pos.x + 5 , tar->pos.y - 5 , tar->pos.x + 15 , tar->pos.y , BLACK );
			gfx_filledTriangle( tar->pos.x - 5 , tar->pos.y + 5 , tar->pos.x - 5 , tar->pos.y - 5 , tar->pos.x - 15 , tar->pos.y , BLACK );
		}
		tar++;


		///////enemy_bullet start
		if( enemy_bul->shot )
        {
			gfx_filledTriangle( enemy_bul->x - 5 , enemy_bul->y , enemy_bul->x + 5 , enemy_bul->y , enemy_bul->x , enemy_bul->y + 20 , RED );
        }
		
		if( enemy_bul->y > gfx_screenHeight() && enemy_bul->shot )//when bullet crosses the higher border of the screen it is no longer shooting
        {
            enemy_bul->shot = 0;
			enemy_bul->period = rand() % 250 + 251;
        }
		enemy_bul++;
		//////enemy_bullet end

	}

	for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )
	{
		if( bul->shot )
        {
			gfx_filledTriangle( bul->x - 5 , bul->y , bul->x + 5 , bul->y , bul->x , bul->y - 20 , RED );
        }
		
		if( bul->y < 0 )//when bullet crosses the higher border of the screen it is no longer shooting
        {
            bul->shot = 0;
        }
		bul++;
	}

}

void score()
{
	gfx_textout( 10 , gfx_screenHeight() - 60 , "Number of killed enemies" , YELLOW );
	gfx_textout( 10 , gfx_screenHeight() - 90 , "Number of fired bullets" , YELLOW);
	gfx_textout( 10 , gfx_screenHeight() - 30 , "Number of remaining lives" , YELLOW);
	sprintf( str , "%d" , kil_num );
	gfx_textout( 230 , gfx_screenHeight() - 60 , str , YELLOW );
	sprintf( str , "%d" , shot_num );
	gfx_textout( 230 , gfx_screenHeight() - 90 , str , YELLOW );
	sprintf( str , "%d" , lives );
	gfx_textout( 230 , gfx_screenHeight() - 30 , str , YELLOW );
	gfx_line( 0 , gfx_screenHeight() - 100 , 285 , gfx_screenHeight() - 100 , YELLOW );
	gfx_line( 285 , gfx_screenHeight() - 100 , 285 , gfx_screenHeight() - 10 , YELLOW );
	gfx_line( 285 , gfx_screenHeight() - 40 , 0 , gfx_screenHeight() - 40 , YELLOW );
	gfx_line( 285 , gfx_screenHeight() - 70 , 0 , gfx_screenHeight() - 70 , YELLOW );
	gfx_line( 220 , gfx_screenHeight() - 100 , 220 , gfx_screenHeight() - 10 , YELLOW );
	gfx_line( 285 , gfx_screenHeight() - 10 , 0 , gfx_screenHeight() - 10 , YELLOW );
}

//new code start
void newLevel( struct Target *tar , struct Bullet *enemy_bul , struct Bullet *bul , int num )
{
	gfx_filledRect(  0 , gfx_screenHeight() - 1 , gfx_screenWidth() - 1 , 0 , BLACK );
	gfx_textout( gfx_screenWidth() / 2 , gfx_screenHeight() / 2 , "NEW LEVEL !" , WHITE );
	gfx_updateScreen();

	/*SDL_Delay(100);

	struct 

	enemy_bul = realloc( enemy_bul , num * sizeof( struct Bullet ) );
	tar = realloc( tar , num * sizeof( struct Target ) );

	free( enemy_bul );
	struct Bullet *enemy_bul = ( struct Bullet *) malloc( num * sizeof( struct Bullet ) );
	free( tar );
	struct Target *tar = ( struct Target *) malloc( num * sizeof( struct Target ) );*/

	for( int i = 0 ; i < NUM_OF_BULLETS ; i++ )
	{
		( bul + i )->shot = 0;
	  	( bul + i )->hit = 0;
	  	( bul + i )->ex_radius = 0;
	}

	for( int i = 0 ; i < num ; i++ )
	{
		( tar + i )->waiting = 1;

		( enemy_bul + i )->shot = 0;
	  	( enemy_bul + i )->hit = 0;
	  	( enemy_bul + i )->ex_radius = 0;
		( enemy_bul + i )->period = rand() % 250 + 251;
	}
}
//new code end