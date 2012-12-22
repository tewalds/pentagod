/**
 * Pentagoo.js
 * Pentagoo class, baby.
 */
 
var Pentagoo = new Class({

	Implements: [Events, Options],

	options:{
		size: 6,					// board size (width and length)
		subboardSize: 3,			// subboard size
		winLength: 5,				// number of straight marbles to indicate winning
		aiURL: '/ai',
		imagesPath: '/images/',
		images: [
			'white-marble.png',
			'black-marble.png',
			'space-select.png',
			'rotate-arrows.png',
			'pentago-subboard-15deg.png',
			'pentago-subboard-30deg.png',
			'pentago-subboard-45deg.png',
			'pentago-subboard-60deg.png',
			'pentago-subboard-75deg.png',
			'marble-select.png',
			'highlight-marble.png',
			'pointer.png'
		]
	},

	initialize: function(options) {
		this.setOptions(options);
		
		this.initStuff();
		this.generateEvents();
		this.preloadStuff();
	},
	
	initStuff: function(){
		
		// Init values for saved/unsaved game variables
		this.boardMatrix = [];
		for(var y=0; y<this.options.size; y++){
			this.boardMatrix[y] = [];
			for(var x=0; x<this.options.size; x++)
				this.boardMatrix[y][x] = 0;
		}
		this.moveHistory = [];
		this.lastMarbleHistory = [];
		this.lastMarble = null;
		this.gameType = 0;
		this.computerLevel = [0,0];
		
		// Default values for variables
		this.game = 0;
		this.currentHistory = null;
		this.boardMatrixCopy = [];
		this.highlightMarble = this.highlightRotate = '00';
		this.boardState = 0;
		this.moveState = 0;

		// Set disabled links
		$('undo-link').addClass('disabled');
		$('history-link').addClass('disabled');

		// Clear all marbles
		$$('.subboard td').setProperty('class','space');

		// Open up the cover
		this.boardCover(false);

		// Set players
		this.player = 1;
		$('player-1-label').addClass('current');
		$('player-2-label').removeClass('current');
		this.playerType = 1;

		// Styles for rotation buttons
		$$('.rotation-buttons').setStyle('opacity', 0);

		// Styles for sub-boards
		$$('.subboard').setStyle('opacity', 1);

		// Styles for panels
		if(!window.ie) $$('.panel').setStyle('opacity', '.85');
		
		this.cmove = [];

		// Clear status
		this.setStatus();
	},
	
	// Generate events
	generateEvents: function(){
		var self = this;
		
		// Links
		$('new-game-link').addEvent('click',function(){
			if(!this.hasClass('disabled')) self.slidePanel('new-game');
		});
		$('cancel-game-link').addEvent('click',function(){
			self.slidePanel('new-game');
		});
		$('start-game-link').addEvent('click',function(){
			self.slidePanel('new-game');
			self.newGame();
		});
		$('undo-link').addEvent('click',function(){
			if(!this.hasClass('disabled')){
				self.slidePanel();
				self.historyBack('undo');
			}
		});
		$('history-link').addEvent('click',function(){
			if(!this.hasClass('disabled')) self.slidePanel('history');
		});
		$('history-start-link').addEvent('click',function(){
			if(!this.disabled) self.historyStart();
		});
		$('history-back-link').addEvent('click',function(){
			if(!this.disabled) self.historyBack();
		});
		$('history-forward-link').addEvent('click',function(){
			if(!this.disabled) self.historyForward();
		});
		$('history-end-link').addEvent('click',function(){
			if(!this.disabled) self.historyEnd();
		});
		$('close-history-link').addEvent('click',function(){
			if(!this.disabled){
				self.historyEnd();
				self.slidePanel('history');
			}
		});
		$('rules-link').addEvent('click',function(){
			if(!this.hasClass('disabled')) self.slidePanel('rules');
		});
		$('close-rules-link').addEvent('click',function(){
			self.slidePanel('rules');
		});
		$('about-link').addEvent('click',function(){
			if(!this.hasClass('disabled')) self.slidePanel('about');
		});
		$('close-about-link').addEvent('click',function(){
			self.slidePanel('about');
		});

		// Board spaces
		$$('.subboard td').addEvent('click',function(){
			if(self.highlightMarble) $('s-'+self.highlightMarble).removeClass('highlight');
			var y = this.id.charAt(2).toInt();
			var x = this.id.charAt(3).toInt();
			self.place(x,y);
			self.highlightMarble = ''+y+x;
		}).addEvent('mousemove',function(){
			if(this.hasClass('space')){
				if(self.highlightMarble) $('s-'+self.highlightMarble).removeClass('highlight');
				var y = this.id.charAt(2);
				var x = this.id.charAt(3);
				self.highlightMarble = ''+y+x;
				$('s-'+self.highlightMarble).addClass('highlight');
			}
		}).addEvent('mouseleave',function(){
			if(self.highlightMarble) $('s-'+self.highlightMarble).removeClass('highlight');
		});

		// Rotation buttons
		$$('.rotate-left').each(function(elem,i){
			elem.addEvent('click',function(){
				self.rotate((i+1),'l');
			});
		});
		$$('.rotate-right').each(function(elem,i){
			elem.addEvent('click',function(){
				self.rotate((i+1),'r');
			});
		});
		$$('.rotate-left','.rotate-right').addEvent('mousemove',function(){
			if(self.highlightRotate) $('rb-'+self.highlightRotate).removeClass('highlight');
			var y = this.id.charAt(3);
			var x = this.id.charAt(4);
			self.highlightRotate = ''+y+x;
			$('rb-'+self.highlightRotate).addClass('highlight');
		}).addEvent('mouseleave',function(){
			if(self.highlightRotate) $('rb-'+self.highlightRotate).removeClass('highlight');
		});

		// Set keyboard shortcuts
		document.addEvent('keydown', function(e){
			var e = new Event(e);
			var arrows = ['up', 'down', 'left', 'right'];

			if(e.key == 'u'){
				e.stop();
				$('undo-link').fireEvent('click');
			}
			else if(arrows.contains(e.key)){
				e.stop();
				if(self.playerType == 1 && !self.game && self.boardState == 0){
					if(self.highlightMarble) $('s-'+self.highlightMarble).removeClass('highlight');
					if(self.highlightRotate) $('rb-'+self.highlightRotate).removeClass('highlight');

					if(self.moveState == 0){
						var y = self.highlightMarble.charAt(0).toInt();
						var x = self.highlightMarble.charAt(1).toInt();

						switch(e.key){
							case arrows[0]: (y==0) ? y=self.options.size-1 : y--; break; // up
							case arrows[1]: (y==self.options.size-1) ? y=0 : y++; break; // down
							case arrows[2]: (x==0) ? x=self.options.size-1 : x--; break; // left
							case arrows[3]: (x==self.options.size-1) ? x=0 : x++; break; // right
						}

						self.highlightMarble = ''+y+x;

						$('s-'+self.highlightMarble).addClass('highlight');
					}
					else{
						var y = self.highlightRotate.charAt(0).toInt();
						var x = self.highlightRotate.charAt(1).toInt();

						switch(e.key){
							case arrows[0]: (y==0) ? y=1 : y--; break; // up
							case arrows[1]: (y==1) ? y=0 : y++; break; // down
							case arrows[2]: (x==0) ? x=3 : x--; break; // left
							case arrows[3]: (x==3) ? x=0 : x++; break; // right
						}

						self.highlightRotate = ''+y+x;

						$('rb-'+self.highlightRotate).addClass('highlight');
					}
				}
			}
			else if(e.key == 'enter'){
				e.stop();
				if(self.playerType == 1 && !self.game && self.boardState == 0){
					if(self.highlightMarble) $('s-'+self.highlightMarble).removeClass('highlight');
					if(self.highlightRotate) $('rb-'+self.highlightRotate).removeClass('highlight');

					if(self.moveState == 0)
						$('s-'+self.highlightMarble).fireEvent('click');
					else
						$('rb-'+self.highlightRotate).fireEvent('click');
				}
			}
		});
	},

	// Preload Stuff
	preloadStuff: function(){
		// Image paths
		var path = this.options.imagesPath;
		var oimages = this.options.images;
		var images = [];
		for(var i=0; i<oimages.length; i++) images.push(path + oimages[i]);

		// Close the cover
		this.boardCover(true);
		
		var self = this;

		// Preload images and loading status
		new Asset.images(images, {
			onProgress: function(i){
				var load_percent = Math.round((i+1)/images.length*100);
				if(i < images.length) self.setStatus('Loading... (' + load_percent + '%)');
			},
			onComplete: function(){
				self.setStatus();
				self.boardCover(false);
			}
		});
	},

	// Slide panels
	slidePanel: function(panel){
		// Remove all 'focus' state first
		$$('#menu a').removeClass('focus');

		// Panel effects
		if(panel){
			var panel_effect = new Fx.Tween(panel, { duration: 200 });

			if($(panel).getStyle('width').toInt() == 0){
				// Add 'focus' to panel's link
				$(panel+'-link').addClass('focus');

				// Close the cover
				this.boardCover(true);

				panel_effect.start('width', 520);
			}
			else{
				// Remove 'focus' to panel's link
				$(panel+'-link').removeClass('focus');

				// Open up the cover if not game
				if(!this.game) this.boardCover(false);

				panel_effect.start('width', 0);
			}
		}

		// Select all panels except the CURRENT panel. Cool.
		var panels = $$('.panel[id!='+panel+']');

		// Styles for all panels
		panels.setStyle('width',0);
	},

	// New game
	newGame: function(){
		// Stop all timers EXCEPT the rotate effects timer
		this.cmove.each(function(move){$clear(move);});

		// Initialization
		this.initStuff();

		// Set default players and game type
		this.gameType = 0;
		$('player-1-type').set('text', 'Human');
		$('player-2-type').set('text', 'Human');

		// Player selection & Computer levels
		if($('p1-c-l').checked){
			this.gameType = 1;
			this.playerType = 2;
			this.computerLevel[0] = $('p1-cl').selectedIndex;
			$('player-1-type').set('text', 'Computer');
			this.computerMove();
		}
		if($('p2-c-l').checked){
			this.computerLevel[1] = $('p2-cl').selectedIndex;
			this.gameType = (this.gameType == 1) ? 2 : 1;
			$('player-2-type').set('text', 'Computer');
		}
	},

	// Place marble
	place: function(x,y){
		var valid_move = this.movePlace(x,y) && this.updateHistory(this.move+y+x);

		if(valid_move){
			// Check win
			this.checkWin();

			if(!this.game && this.playerType == 1){
				// Show rotation buttons
				this.rotateButtons(1);

				// Enable undo link if history is not empty
				if(this.moveHistory.length && this.gameType == 0)
					$('undo-link').removeClass('disabled');
			}
		}
	},

	// Place move
	movePlace: function(x,y){
		this.move = 'p';
		var space = $('s-'+y+x);

		if(this.boardMatrix[y][x] == 0){
			// Remove last highlighted marble
			if(this.lastMarble) $('s-'+this.lastMarble).removeClass('last');

			// Add the marble for current player
			this.boardMatrix[y][x] = this.player;
			space.className = 'p'+this.player;

			// Indicate last marble
			this.lastMarble = ''+y+x;
			space.addClass('last');

			return true;
		}

		return false;
	},

	// Unplace move - opposite of movePlace
	moveUnplace: function(x,y){
		var space = $('s-'+y+x);

		if(this.boardMatrix[y][x] != 0){
			// Remove the marble for current player
			this.boardMatrix[y][x] = 0;
			space.className = 'space';

			// Indicate last marble, using this.currentHistory
			this.lastMarble = this.lastMarbleHistory[this.currentHistory-1];
			if(this.lastMarble) $('s-'+this.lastMarble).addClass('last');

			return true;
		}

		return false;
	},

	// Rotate sub-board
	rotate: function(subboard, direction){
		// Close the cover
		this.boardCover(true);

		// Disable undo link
		$('undo-link').addClass('disabled');

		// Hide rotation buttons
		if(this.playerType == 1) this.rotateButtons(0);

		var time = this.moveRotate(subboard, direction);

		(function(){
			// Update history
			var valid_move = this.updateHistory(this.move+subboard+direction);

			if(valid_move){
				// Check win
				this.checkWin();

				if(!this.game){
					// Switch player
					this.switchPlayer();

					if(this.playerType == 1){
						if(this.gameType == 0){
							// Open up the cover
							this.boardCover(false);

							// Enable undo link
							$('undo-link').removeClass('disabled');
						}
						else if(this.gameType == 1){
							this.playerType = 2;
							this.computerMove();
						}
					}
					else if(this.playerType == 2){
						if(this.gameType == 1){
							this.playerType = 1;

							// Open up the cover
							this.boardCover(false);
						}
						else if(this.gameType == 2) this.computerMove();
					}
				}
			}
		}.bind(this)).delay(time);
	},

	// Rotate move
	moveRotate: function(subboard, direction){
		this.move = 'r';
		var matrix = []; // stores sub-board matrix
		var init_last = false; // flag to specify initialized last marble

		// Shift coordinates for specific subboards
		var sx = (subboard == 2 || subboard == 4) ? this.options.subboardSize : 0;
		var sy = (subboard == 3 || subboard == 4) ? this.options.subboardSize : 0;

		// Extract the subboard matrix
		for(var y=0; y<this.options.subboardSize; y++){
			matrix[y] = [];
			for(var x=0; x<this.options.subboardSize; x++)
				matrix[y][x] = this.boardMatrix[y+sy][x+sx];
		}

		// Rotate and put back into the board matrix, also rotate the last marble
		if(direction == 'r'){
			for(var y=0 ; y<this.options.subboardSize ; y++ )
				for(var x=0 ; x<this.options.subboardSize ; x++ ){
					this.boardMatrix[y+sy][x+sx] = matrix[(2-x)][y];

					if(this.lastMarble == ''+(2-x+sy)+(y+sx) && !init_last){
						this.lastMarble = ''+(y+sy)+(x+sx);
						init_last = true;
					}
				}
		}
		else if(direction == 'l'){
			for(var y=0 ; y<this.options.subboardSize ; y++ )
				for(var x=0 ; x<this.options.subboardSize ; x++ ){
					this.boardMatrix[y+sy][x+sx] = matrix[x][(2-y)];

					if(this.lastMarble == ''+(x+sy)+(2-y+sx) && !init_last){
						this.lastMarble = ''+(y+sy)+(x+sx);
						init_last = true;
					}
				}
		}

		return this.subboardRotationFx(subboard,direction);
	},

	// Unrotate move - opposite of moveRotate
	moveUnrotate: function(subboard, direction){
		// Reverse the rotation direction
		var reverse_direction = (direction == 'l') ? 'r' : 'l';

		return this.moveRotate(subboard, reverse_direction);
	},

	// Sub-board rotation effects
	subboardRotationFx: function(subboard, direction){
		var div = $('sb-'+subboard).getParent(); // parent element of the sub-board
		var FRAMES = 6; // number of rotation frames
		var PERIOD = 32; // frame period
		var opac; // opacity
		var mid_frame = FRAMES/2; // mid frame

		// Shift coordinates for specific quadrants
		var sx = (subboard == 2 || subboard == 4) ? this.options.subboardSize : 0;
		var sy = (subboard == 3 || subboard == 4) ? this.options.subboardSize : 0;

		// Sub-board rotation effects
		var k = 1;
		var rotate_bg = (function(){
			if(k == FRAMES){
				$clear(rotate_bg);

				// Clear subboard rotation image
				div.className = '';

				// Set back the subboard opacity
				$('sb-'+subboard).setStyle('opacity', 1);

				return;
			}
			else{
				// Set subboard rotation images
				div.className = 'subboard-' + ((direction == 'l') ? k : FRAMES-k);

				// Set opacity for sub-board (fading effect)
				opac = Math.abs(mid_frame-k)/mid_frame;
				$('sb-'+subboard).setStyle('opacity', opac);

				// Rotate the marbles during half-time of animation
				if(k == Math.round(mid_frame)){
					for(var y=sy; y<this.options.subboardSize+sy; y++)
						for(var x=sx; x<this.options.subboardSize+sx; x++)
							$('s-'+y+x).className = (this.boardMatrix[y][x]) ? 'p' + this.boardMatrix[y][x] : 'space';
					// Indicate last marble
					if(this.lastMarble) $('s-'+this.lastMarble).addClass('last');
				}
			}

			k++;

		}.bind(this)).periodical(PERIOD);

		return PERIOD*FRAMES;
	},

	// Update history list after move and validation
	updateHistory: function(move_type){
		var last_index = this.moveHistory.length;
		var this_move = move_type.charAt(0);

		// The validation:
		// 1. Validate the moves
		//    a. history index is EVEN (0,2,4...) and the move is PLACE (p)
		//    b. history index is ODD (1,3,5...) and the move is ROTATE (r)
		// 2. Validate the current player, with the following algorithm:
		//    this.moveHistory index:    01 23 45 67 89  <-- last_index
		//    even_index        :    00 22 44 66 88
		//    linear_index      :    00 11 22 33 44
		//    player            :    11 22 11 22 11  <-- (last) current player
		var even_index = last_index - (last_index%2);
		var linear_index = even_index/2;
		var current_player = (linear_index%2) ? 2 : 1;

		// Start validation
		if(((last_index%2 == 0 && this_move == 'p') || (last_index%2 != 0 && this_move == 'r'))
			&& this.player == current_player){
			// Update move history
			this.moveHistory.push(move_type);

			// Update last marble history
			this.lastMarbleHistory.push(this.lastMarble);

			return true;
		}
		// Invalidity found
		else{
			// Close the cover
			this.boardCover(true);

			// Disable undo link
			$('undo-link').addClass('disabled');

			// Error status
			this.setStatus('Sorry, an error has occured. Please start a new game.');

			return false;
		}
	},

	// Switch players
	switchPlayer: function(){
		var prevPlayer = this.player;
		this.player = (this.player == 1) ? 2 : 1;
		$('player-'+prevPlayer+'-label').removeClass('current');
		$('player-'+this.player+'-label').addClass('current');
	},

	// Toggle rotation buttons
	rotateButtons: function(show){
		var rotationButton = $$('.rotation-buttons');
		var opac = rotationButton[0].getStyle('opacity');

		if(show && opac == 0){
			rotationButton.setStyle('opacity', '.4');
			this.moveState = 1;
		}
		else if(!show && opac > 0){
			rotationButton.setStyle('opacity', 0);
			this.moveState = 0;
		}
	},

	// Open/Close the cover on the board
	boardCover: function(state){
		var cover = $('board-cover');

		cover.setStyle('z-index', (state) ? 100 : 0);
		this.boardState = (state) ? 1 : 0;
	},

	// The starting point of history
	historyStart: function(){
		if(this.currentHistory != -1){
			// Set pointer to history
			this.currentHistory = -1;

			// Clear status
			this.setStatus();

			// Disable all other menu links except history link
			$$('#menu a[id!=history-link]').addClass('disabled');

			// Back from a win-state game
			if(this.game){
				this.game = 0;

				// Backup board matrix
				if(!this.boardMatrixCopy.length)
					for(var y=0; y<this.options.size; y++){
						this.boardMatrixCopy[y] = [];
						for(var x=0; x<this.options.size; x++)
							this.boardMatrixCopy[y][x] = this.boardMatrix[y][x];
					}
			}

			// Clear board matrix and all marbles
			for(var y=0; y<this.options.size; y++)
				for(var x=0; x<this.options.size; x++)
					this.boardMatrix[y][x] = 0;

			// Clear all marbles
			$$('.subboard td').setProperty('class','space');

			// Set current player
			this.player = 1;
			$('player-1-label').addClass('current');
			$('player-2-label').removeClass('current');

			// Update visual history pointer
			this.updateHistoryPointer();
		}
	},

	// Back to the past (in history), including undo
	historyBack: function(action){
		// Set pointer to history
		var current_action;
		if(action == 'undo'){
			this.currentHistory = this.moveHistory.length-1; // to be used for move_unplace
			current_action = this.moveHistory.getLast();
		}
		else{
			if(this.currentHistory == null) this.currentHistory = this.moveHistory.length-1;
			current_action = this.moveHistory[this.currentHistory];
		}

		if(current_action && this.currentHistory != null){
			var prev_game;

			// Clear status
			this.setStatus();

			// Disable undo link
			$('undo-link').addClass('disabled');

			if(!$defined(action)){
				// Disable history buttons
				$$('.history-buttons button').setProperty('disabled','disabled');

				// Disable all other menu links except history link
				$$('#menu a[id!=history-link]').addClass('disabled');
			}

			// Get move properties
			var move_type = current_action.split('');
			var this_move = move_type[0];
			var move_action1 = move_type[1];
			var move_action2 = move_type[2];

			// Back from a win-state game
			if(this.game){
				this.game = 0;
				prev_game = 1;

				// Remove all winning marbles
				$$('.subboard td').removeClass('win');

				if(action == "undo"){
					// Open up the cover
					this.boardCover(false);

					// Disable history link
					 $('history-link').addClass('disabled');
				}
				// Backup board matrix
				else if(!this.boardMatrixCopy.length){
					for(var y=0; y<this.options.size; y++){
						this.boardMatrixCopy[y] = [];
						for(var x=0; x<this.options.size; x++)
							this.boardMatrixCopy[y][x] = this.boardMatrix[y][x];
					}
				}

				// Set current player because when 'game', current player is removed
				$('player-'+this.player+'-label').addClass('current');
			}

			// Execute the 'un'-move
			if(this_move == 'p'){
				var y = move_action1.toInt();
				var x = move_action2.toInt();

				var valid_move = this.moveUnplace(x,y);

				if(valid_move){
					if(action == 'undo'){
						// Hide rotation buttons
						this.rotateButtons(0);

						// Enable undo link
						$('undo-link').removeClass('disabled');
					}
					else
						// Enable history buttons
						$$('.history-buttons button').removeProperty('disabled');
				}
			}
			else if(this_move == 'r'){
				var t = move_action1.toInt();
				var d = move_action2;

				var time = this.moveUnrotate(t,d);

				(function(){
					// Switch player
					if(!prev_game) this.switchPlayer();

					if(action == 'undo'){
						// Show rotation buttons
						this.rotateButtons(1);

						// Enable undo link
						$('undo-link').removeClass('disabled');
					}
					else
						// Enable history buttons
						$$('.history-buttons button').removeProperty('disabled');
				}.bind(this)).delay(time);
			}
			
			if(action == "undo"){
				// Clear history pointer
				this.currentHistory = null;

				// Remove last history item after undo
				this.moveHistory.pop();
				this.lastMarbleHistory.pop();
				
				// Disable undo link if history is empty
				if(!this.moveHistory.length) $('undo-link').addClass('disabled');
			}
			// Update pointer to history list after back
			else if(this.currentHistory >= 0)
				this.currentHistory--;

			// Update visual history pointer
			this.updateHistoryPointer();
		}
	},

	// Forward to the future (in history)
	historyForward: function(){
		// Sets pointer to history
		var last_index = this.moveHistory.length-1;
		if(this.currentHistory != null && this.currentHistory < last_index) this.currentHistory++;
		var current_action = this.moveHistory[this.currentHistory];

		if(current_action && this.currentHistory != null){
			// Clear status
			this.setStatus();

			// Disable all other menu links except history link
			$$('#menu a[id!=history-link]').addClass('disabled');

			// Disables history buttons
			$$('.history-buttons button').setProperty('disabled','disabled');

			// Get move properties
			var move_type = current_action.split('');
			var this_move = move_type[0];
			var move_action1 = move_type[1];
			var move_action2 = move_type[2];

			// Executes the move
			if(this_move == 'p'){
				var y = move_action1.toInt();
				var x = move_action2.toInt();

				var valid_move = this.movePlace(x,y);

				if(valid_move){
					// Check win
					this.checkWin();

					// Clear history pointer
					if(this.game) this.currentHistory = null;

					// Enable history buttons
					$$('.history-buttons button').removeProperty('disabled');
				}
			}
			else if(this_move == 'r'){
				var t = move_action1.toInt();
				var d = move_action2;

				var time = this.moveRotate(t,d);

				(function(){
					// Check win
					this.checkWin();

					// Clear history pointer
					if(this.game) this.currentHistory = null;
					// Switch player
					else this.switchPlayer();

					// Enable history buttons
					$$('.history-buttons button').removeProperty('disabled');
				}.bind(this)).delay(time);
			}

			// Update visual history pointer
			this.updateHistoryPointer();
		}
	},

	// The ending point of history
	historyEnd: function(){
		if(this.boardMatrixCopy.length){
			// Copy back the original matrix
			for(var y=0; y<this.options.size; y++)
				for(var x=0; x<this.options.size; x++){
					this.boardMatrix[y][x] = this.boardMatrixCopy[y][x];
					$('s-'+y+x).className = this.boardMatrix[y][x] ? 'p' + this.boardMatrix[y][x] : 'space';
				}

			// Clear the matrix copy
			this.boardMatrixCopy = [];

			// Clear history pointer
			this.currentHistory = null;

			// Check win so that winning marbles are highlighted
			// move is assigned so that a draw can be determined as well
			this.move = (this.moveHistory.getLast().charAt(0) == 'r') ? 'r' : 'p';
			this.checkWin();

			// Enable all menu links including undo link if no computers
			$$('#menu a').removeClass('disabled');
			if(this.gameType>0) $('undo-link').addClass('disabled');

			// Set back current player using the algorithm from updateHistory
			var last_index = this.moveHistory.length-1;
			var even_index = last_index - (last_index%2);
			var linear_index = even_index/2;
			this.player = (linear_index%2) ? 2 : 1;

			// Update visual history pointer
			this.updateHistoryPointer();
		}
	},

	// Update visual pointer to history bar
	updateHistoryPointer: function(){
		var historyPointer = $('history-pointer');
		if(this.currentHistory == null)
			historyPointer.setStyle('left','100%');
		else{
			var pointer_position = (this.currentHistory+1)/this.moveHistory.length * 100;
			historyPointer.setStyle('left',pointer_position+'%');
		}
	},

	// History play
	historyPlay: function(state){
		var PERIOD = 1000;

		if(state && this.currentHistory != null)
			this.playSteps = this.historyForward.periodical(PERIOD);
		else
			$clear(this.playSteps);
	},

	// Check winnnings or draws
	checkWin: function(){
		var check_points = this.options.size - this.options.winLength + 1;

		// Check all horizontal and vertical wins
		for(var i=0; i<this.options.size; i++)
			for(var j=0; j<check_points; j++){
				this.checkWinningMarbles(j,i,'horizontal');
				this.checkWinningMarbles(i,j,'vertical');
			}

		// Check diagonal wins
		for(var k=0; k<check_points; k++)
			for(var l=0; l<check_points; l++){
				this.checkWinningMarbles(k,l,'l-diagonal');
				this.checkWinningMarbles((k+4),l,'r-diagonal');
			}

		// Check for draw game, after the player rotates
		var draw = (this.move == 'r' && !this.boardMatrix.toString().contains(0)) ? 1 : 0;
		if(draw && !this.game) this.game = 4;

		// Update game status
		var status;
		switch(this.game){
			case 1: status = 'Player 1 wins!'; break;
			case 2: status = 'Player 2 wins!'; break;
			case 3: status = 'Player 1 and 2 wins! A draw?'; break;
			case 4: status = 'It\'s a draw!'; break;
		}

		if(this.game){
			// Display game status
			this.setStatus(status);

			// Close the cover
			this.boardCover(true);

			// Remove last highlighted marble
			if(this.lastMarble) $('s-'+this.lastMarble).removeClass('last');

			// Hide current player states
			$('player-2-label').removeClass('current');
			$('player-1-label').removeClass('current');

			// Enable history link
			$('history-link').removeClass('disabled');
			
			// Enable undo link
			if(this.gameType == 0) $('undo-link').removeClass('disabled');
		}
	},

	// Check validity for 5 straight marbles
	checkWinningMarbles: function(x,y, direction){
		var valid = false; // flag for valid straight same marbles
		var state = this.boardMatrix[y][x];

		if(state){
			// Check for all directions
			switch(direction){
				case 'horizontal':
					for(var i=1; i<this.options.winLength && (valid = this.boardMatrix[y][x+i] == state); i++);

					if(valid)
						for(var j=x; j<x+this.options.winLength; j++)
							$('s-'+y+j).addClass('win');
					break;

				case 'vertical':
					for(var i=1; i<this.options.winLength && (valid = this.boardMatrix[y+i][x] == state); i++);

					if(valid)
						for(var j=y; j<y+this.options.winLength; j++)
							$('s-'+j+x).addClass('win');
					break;

				case 'l-diagonal':
					for(var i=1; i<this.options.winLength && (valid = this.boardMatrix[y+i][x+i] == state); i++);

					if(valid)
						for(var j=y, k=x; j<y+this.options.winLength && k<x+this.options.winLength; j++, k++)
							$('s-'+j+k).addClass('win');
					break;

				case 'r-diagonal':
					for(var i=1; i<this.options.winLength && (valid = this.boardMatrix[y+i][x-i] == state); i++);

					if(valid)
						for(var j=y, k=x; j<y+this.options.winLength && k>x-this.options.winLength; j++, k--)
							$('s-'+j+k).addClass('win');
					break;
			}

			if(valid){
				// Define the winning game
				if(state == 1)
					this.game = (this.game == 2) ? 3 : 1;
				else if(state == 2)
					this.game = (this.game == 1) ? 3 : 2;
			}
		}
	},

	// Status display
	setStatus: function(text){
		var status = $('status');
		if($defined(text))
			status.set('text', text).fade(1);
		else
			status.empty().fade(0);
	},

	// Computer move
	computerMove: function(){
		var TIME = 1000;

		// Close the cover
		this.boardCover(true);
		
		if(this.playerType == 2){
			//convert history to xytd format, same as expected as a response
			var moves = [];
			var hist = this.moveHistory;
			for(var i = 0; i < hist.length; i+=2) {
				moves.push(hist[i].charAt(2) + hist[i].charAt(1) + hist[i+1].substring(1,3));
			}

			// AI parameters
			ai_parameters = {
				'hist': moves.join(' '),
				'l': this.computerLevel[this.player-1],
			};
			
			// Request AI's moves
			var request = new Request({
				url: this.options.aiURL,
				method: 'get',
				data: ai_parameters,
				onComplete: function(response){
					var rmove = response.split(''); //x,y,t,d

					this.cmove[0] = (function(){
						// Computer place
						this.place(rmove[0].toInt(),rmove[1].toInt()); // x,y

						if(!this.game)
							this.cmove[1] = (function(){
								// Computer rotate
								this.rotate(rmove[2].toInt(),rmove[3]); // t,d
							}.bind(this)).delay(TIME,rmove);
					}.bind(this)).delay(TIME,rmove);
				}.bind(this)
			}).send();
		}
	}
	
});
