
#include "utils.h"
#include "video.h"


void MenuPrepareToDraw()
{
	// clear the screen, buffer Z and initializes environment to 2D
	tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

	// Enable alpha Test
	tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);

	// Enable alpha blending.
	tiny3d_BlendFunc(1, (blend_src_func)(TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA),
		(blend_dst_func)(NV30_3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | NV30_3D_BLEND_FUNC_DST_ALPHA_ZERO),
		(blend_func)(TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD));
}

void MenuFillBackground( u32 rgba )
{
	tiny3d_SetPolygon( TINY3D_QUADS );

	tiny3d_VertexPos( 0, 0, 65535 );
	tiny3d_VertexColor( rgba );

	tiny3d_VertexPos( WINDOW_WIDTH, 0, 65535 );
	tiny3d_VertexPos( WINDOW_WIDTH, WINDOW_HEIGHT, 65535 );
	tiny3d_VertexPos( 0  , WINDOW_HEIGHT, 65535 );
	tiny3d_End();
}

//yet untested
void MenuDrawRectangleRot( u32 rgba, float width, float height, float x, float y, float z, u8 filled, float angleZ, float scale )
{
	//vars

	float dx = ( width / 2 ) * scale;
	float dy = ( height / 2 ) * scale;

	MATRIX matrix;

	float angle = -( PI * angleZ )/180.0f;

	// rotate and translate the sprite
	matrix = MatrixRotationZ( angle );
	matrix = MatrixMultiply( matrix, MatrixTranslation(x + dx, y + dy, 0.0f ) );

	VECTOR tr;
	VECTOR tl;
	VECTOR bl;
	VECTOR br;

	tl.x = -dx;
	tl.y = -dy;
	tl.z = z;

	//assign all other corners based on the top left one
	tr.x = dx;
	tr.y = -dy;
	tr.z = z;

	br.x = dx;
	br.y = dy;
	br.z = z;

	bl.x = -dx;
	bl.y = dy;
	bl.z = z;

	//start drawing
	if( filled )
		tiny3d_SetPolygon( TINY3D_QUADS );
	else
		tiny3d_SetPolygon( TINY3D_LINE_LOOP );

	tiny3d_VertexPosVector( tl );
	tiny3d_VertexColor( rgba );
	tiny3d_VertexPosVector( tr );
	tiny3d_VertexPosVector( br );
	tiny3d_VertexPosVector( bl );

	tiny3d_End();
	tiny3d_SetMatrixModelView( (MATRIX*)NULL); // set matrix identity
}

void MenuDrawRectangle( u32 rgba, float width, float height, float x, float y, float z, u8 filled, float angleZ, float scale )
{
	//printf("MenuDrawRectangle( %08x, %.2f, %.2f, %.2f, %.2f, %.2f, %u )\n", rgba, width, height, x, y, z, filled);
	if( angleZ )
	{
		MenuDrawRectangleRot( rgba,  width, height, x, y, z, filled, angleZ, scale );
		return;
	}
	//vars
	VECTOR tr;
	VECTOR tl;
	VECTOR bl;
	VECTOR br;

	float dx = x + width;
	float dy = y + height;

	if( scale != 1.0f )
	{
		float difX = ( ( width * scale ) - width ) / 2.0f;
		dx += difX;
		x -= difX;

		float difY = ( ( height * scale ) - height ) / 2.0f;
		dy += difY;
		y -= difY;
	}

	tl.x = x;
	tl.y = y;
	tl.z = z;

	//assign all other corners based on the top left one
	tr.x = dx;
	tr.y = y;
	tr.z = z;

	br.x = dx;
	br.y = dy;
	br.z = z;

	bl.x = x;
	bl.y = dy;
	bl.z = z;

	//start drawing
	if( filled )
		tiny3d_SetPolygon( TINY3D_QUADS );
	else
		tiny3d_SetPolygon( TINY3D_LINE_LOOP );

	tiny3d_VertexPosVector( tl );
	tiny3d_VertexColor( rgba );
	tiny3d_VertexPosVector( tr );
	tiny3d_VertexPosVector( br );
	tiny3d_VertexPosVector( bl );

	tiny3d_End();
}

void MenuDrawImageRot( u32 rsxOffset, u32 format, float width, float height, u32 wpitch, float x, float y, float z, float angleZ, u8 alpha, float scale )
{
	//printf("MenuDrawImageRot( %08x, %.2f, %.2f, %08x, %.2f, %.2f, %.2f, %.2f )\n",rsxOffset, width, height, wpitch, x, y, z, angleZ );
	float dx = ( width / 2 ) * scale;
	float dy = ( height / 2 ) * scale;

	tiny3d_SetTexture( 0, rsxOffset, width, height, wpitch, (text_format)format, TEXTURE_LINEAR );

	MATRIX matrix;

	float angle = -( PI * angleZ )/180.0f;

	// rotate and translate the sprite
	matrix = MatrixRotationZ( angle );
	matrix = MatrixMultiply( matrix, MatrixTranslation(x + dx, y + dy, 0.0f ) );


	// fix ModelView Matrix
	tiny3d_SetMatrixModelView( &matrix );

	tiny3d_SetPolygon( TINY3D_QUADS );

	tiny3d_VertexPos( -dx, -dy, z );
	tiny3d_VertexColor( 0xffffff00 | alpha );
	tiny3d_VertexTexture( 0.0f ,0.0f );

	tiny3d_VertexPos(dx , -dy, z );
	tiny3d_VertexTexture( 1, 0.0f );

	tiny3d_VertexPos(dx , dy , z );
	tiny3d_VertexTexture( 1, 1 );

	tiny3d_VertexPos(-dx, dy , z );
	tiny3d_VertexTexture( 0.0f ,1 );
	tiny3d_End();

	tiny3d_SetMatrixModelView((MATRIX*)NULL); // set matrix identity
}

void MenuDrawImage( u32 rsxOffset, u32 format, float width, float height, u32 wpitch, float x, float y, float z, float angleZ, u8 alpha, float scale )
{
	if( angleZ )
	{
		MenuDrawImageRot( rsxOffset, format, width, height, wpitch, x, y, z, angleZ, alpha, scale );
		return;
	}

	tiny3d_SetTexture( 0, rsxOffset, width, height, wpitch, (text_format)format, TEXTURE_LINEAR );

	tiny3d_SetPolygon( TINY3D_QUADS );

	float dx = x + width;
	float dy = y + height;

	if( scale != 1.0f )
	{
		float difX = ( ( width * scale ) - width ) / 2.0f;
		dx += difX;
		x -= difX;

		float difY = ( ( height * scale ) - height ) / 2.0f;
		dy += difY;
		y -= difY;
	}

	tiny3d_VertexPos( x, y, z );
	tiny3d_VertexColor( 0xffffff00 | alpha );
	tiny3d_VertexTexture( 0.0f, 0.0f );

	tiny3d_VertexPos( dx, y, z );
	tiny3d_VertexTexture(1, 0.0f);

	tiny3d_VertexPos( dx, dy, z );
	tiny3d_VertexTexture(1, 1);

	tiny3d_VertexPos( x, dy, z );
	tiny3d_VertexTexture(0.0f, 1);

	tiny3d_End();
}

void AdjustViewport( float x, float y )
{
	double sx = (double) Video_Resolution.width;
	double sy = (double) Video_Resolution.height;
	double px = (double) (1000 + x)/1000.0;
	double py = (double) (1000 + y)/1000.0;

	tiny3d_UserViewport( 1,
						(float) ((sx - sx * px) / 2.0), // 2D position
						(float) ((sy - sy * py) / 2.0),
						(float) ((sx * px) / 848.0),    // 2D scale
						(float) ((sy * py) / 512.0),
						(((float) Video_Resolution.width) / 1920.0f) * (float) (1000 + x)/1000.0f,  // 3D scale
						(((float) Video_Resolution.height) / 1080.0f) * (float) (1000 + y)/1000.0f);



}

