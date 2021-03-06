// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('ntp4', function() {
  'use strict';

  /**
   * Creates a new Tileobject. Tiles wrap content on a TilePage, providing
   * some styling and drag functionality.
   * @constructor
   * @extends {HTMLDivElement}
   */
  function Tile(contents) {
    var tile = cr.doc.createElement('div');
    tile.__proto__ = Tile.prototype;
    tile.initialize(contents);

    return tile;
  }

  Tile.prototype = {
    __proto__: HTMLDivElement.prototype,

    initialize: function(contents) {
      // 'real' as opposed to doppleganger.
      this.className = 'tile real';
      this.appendChild(contents);
      contents.tile = this;

      this.addEventListener('dragstart', this.onDragStart_);
      this.addEventListener('drag', this.onDragMove_);
      this.addEventListener('dragend', this.onDragEnd_);

      this.addEventListener('webkitTransitionEnd', this.onTransitionEnd_);
    },

    get index() {
      return Array.prototype.indexOf.call(this.parentNode.children, this);
    },

    /**
     * Position the tile at |x, y|, and store this as the grid location, i.e.
     * where the tile 'belongs' when it's not being dragged.
     * @param {number} x The x coordinate, in pixels.
     * @param {number} y The y coordinate, in pixels.
     */
    setGridPosition: function(x, y) {
      this.gridX = x;
      this.gridY = y;
      this.moveTo(x, y);
    },

    /**
     * Position the tile at |x, y|.
     * @param {number} x The x coordinate, in pixels.
     * @param {number} y The y coordinate, in pixels.
     */
    moveTo: function(x, y) {
      this.style.left = x + 'px';
      this.style.top = y + 'px';
    },

    /**
     * The handler for dragstart events fired on |this|.
     * @param {Event} e The event for the drag.
     * @private
     */
    onDragStart_: function(e) {
      TilePage.currentlyDraggingTile = this;

      e.dataTransfer.effectAllowed = 'copyMove';
      // TODO(estade): fill this in.
      e.dataTransfer.setData('text/plain', 'foo');

      this.startScreenX = e.screenX;
      this.startScreenY = e.screenY;

      this.classList.add('dragging');
    },

    /**
     * The handler for drag events fired on |this|.
     * @param {Event} e The event for the drag.
     * @private
     */
    onDragMove_: function(e) {
      var diffX = e.screenX - this.startScreenX;
      var diffY = e.screenY - this.startScreenY;
      this.moveTo(this.gridX + diffX, this.gridY + diffY);
    },

    /**
     * The handler for dragend events fired on |this|.
     * @param {Event} e The event for the drag.
     * @private
     */
    onDragEnd_: function(e) {
      TilePage.currentlyDraggingTile = null;
      this.classList.remove('dragging');
      // This class is required for the tile to animate to its final position.
      this.classList.add('placing');
      this.tilePage.positionTile_(this.index);
    },

    /**
     * Creates a clone of this node offset by the coordinates. Used for the
     * dragging effect where a tile appears to float off one side of the grid
     * and re-appear on the other.
     * @param {number} x x-axis offset, in pixels.
     * @param {number} y y-axis offset, in pixels.
     */
    showDoppleganger: function(x, y) {
      // We always have to clear the previous doppleganger to make sure we get
      // style updates for the contents of this tile.
      this.clearDoppleganger();

      var clone = this.cloneNode(true);
      clone.classList.remove('real');
      clone.classList.add('doppleganger');
      var clonelets = clone.querySelectorAll('.real');
      for (var i = 0; i < clonelets.length; i++) {
        clonelets[i].classList.remove('real');
      }

      this.appendChild(clone);
      this.doppleganger_ = clone;

      this.doppleganger_.style.WebkitTransform = 'translate(' + x + 'px, ' +
                                                                y + 'px)';
    },

    /**
     * Destroys the current doppleganger.
     */
    clearDoppleganger: function() {
      if (this.doppleganger_) {
        this.removeChild(this.doppleganger_);
        this.doppleganger_ = null;
      }
    },

    /**
     * When a positioning transition ends, remove the 'placing' class so further
     * positioning won't necessarily be animated.
     * @param {Event} e The transition end event.
     */
    onTransitionEnd_: function(e) {
      if (e.propertyName == 'top' || e.propertyName == 'left')
        this.classList.remove('placing');
    }
  };

  /**
   * Gives the proportion of the row width that is devoted to a single icon.
   * @param {number} rowTileCount The number of tiles in a row.
   * @return {number} The ratio between icon width and row width.
   */
  function tileWidthFraction(rowTileCount) {
    return rowTileCount +
        (rowTileCount - 1) * TILE_SPACING_FRACTION;
  }

  /**
   * Calculates an assortment of tile-related values for a grid with the
   * given dimensions.
   * @param {number} width The pixel width of the grid.
   * @param {number} numRowTiles The number of tiles in a row.
   * @return {Object} A mapping of pixel values.
   */
  function tileValuesForGrid(width, numRowTiles) {
    var tileWidth = width / tileWidthFraction(numRowTiles);
    var offsetX = tileWidth * (1 + TILE_SPACING_FRACTION);
    var interTileSpacing = offsetX - tileWidth;

    return {
      tileWidth: tileWidth,
      offsetX: offsetX,
      interTileSpacing: interTileSpacing,
    };
  }

  // The proportion of the tile width which will be used as spacing between
  // tiles.
  var TILE_SPACING_FRACTION = 1 / 8;

  // The smallest amount of horizontal blank space to display on the sides when
  // displaying a wide arrangement.
  var MIN_WIDE_MARGIN = 100;

  /**
   * Creates a new TilePage object. This object contains tiles and controls
   * their layout.
   * @param {string} name The display name for the page.
   * @param {Object} gridValues Pixel values that define the size and layout
   *     of the tile grid.
   * @constructor
   * @extends {HTMLDivElement}
   */
  function TilePage(name, gridValues) {
    var el = cr.doc.createElement('div');
    el.pageName = name;
    el.gridValues_ = gridValues;
    el.__proto__ = TilePage.prototype;
    el.initialize();

    return el;
  }

  /**
   * Takes a collection of grid layout pixel values and updates them with
   * additional tiling values that are calculated from TilePage constants.
   * @param {Object} grid The grid layout pixel values to update.
   */
  TilePage.initGridValues = function(grid) {
    // The amount of space we need to display a narrow grid (all narrow grids
    // are this size).
    grid.narrowWidth =
        grid.minTileWidth * tileWidthFraction(grid.minColCount);
    // The minimum amount of space we need to display a wide grid.
    grid.minWideWidth =
        grid.minTileWidth * tileWidthFraction(grid.maxColCount);
    // The largest we will ever display a wide grid.
    grid.maxWideWidth =
        grid.maxTileWidth * tileWidthFraction(grid.maxColCount);
    // Tile-related pixel values for the narrow display.
    grid.narrowTileValues = tileValuesForGrid(grid.narrowWidth,
                                              grid.minColCount);
    // Tile-related pixel values for the minimum narrow display.
    grid.wideTileValues = tileValuesForGrid(grid.minWideWidth,
                                            grid.maxColCount);
  },

  // We can't pass the currently dragging tile via dataTransfer because of
  // http://crbug.com/31037
  TilePage.currentlyDraggingTile = null;

  TilePage.prototype = {
    __proto__: HTMLDivElement.prototype,

    initialize: function() {
      this.className = 'tile-page';

      var title = this.ownerDocument.createElement('span');
      title.textContent = this.pageName;
      title.className = 'tile-page-title';
      this.appendChild(title);

      // Div that holds the tiles.
      this.tileGrid_ = this.ownerDocument.createElement('div');
      this.tileGrid_.className = 'tile-grid';
      this.appendChild(this.tileGrid_);

      // Ordered list of our tiles.
      this.tileElements_ = this.tileGrid_.getElementsByClassName('tile real');

      this.maskWidth_ = this.clientWidth;

      this.eventTracker = new EventTracker();
      this.eventTracker.add(window, 'resize', this.onResize_.bind(this));

      this.tileGrid_.addEventListener('dragenter',
                                      this.onDragEnter_.bind(this));
      this.tileGrid_.addEventListener('dragover', this.onDragOver_.bind(this));
      this.tileGrid_.addEventListener('drop', this.onDrop_.bind(this));
      this.tileGrid_.addEventListener('dragleave',
                                      this.onDragLeave_.bind(this));
    },

    /**
     * Cleans up resources that are no longer needed after this TilePage
     * instance is removed from the DOM.
     */
    tearDown: function() {
      this.eventTracker.removeAll();
    },

    /**
     * @protected
     */
    appendTile: function(tileElement) {
      var wrapperDiv = new Tile(tileElement);
      wrapperDiv.tilePage = this;
      this.tileGrid_.appendChild(wrapperDiv);

      this.positionTile_(this.tileElements_.length - 1);
      this.classList.remove('animating-tile-page');

      // This would be in initialize(), but at that point we don't yet know our
      // width. This won't do any work if it doesn't need to update the mask.
      this.updateMask_();
    },

    /**
     * Makes some calculations for tile layout. These calculations are shared
     * by |positionTile_| and |getWouldBeIndexforPoint_|.
     * @return {Object} Assorted layout pixel values.
     * @private
     */
    calculateLayoutValues_: function() {
      var grid = this.gridValues_;
      var availableSpace = this.tileGrid_.clientWidth - 2 * MIN_WIDE_MARGIN;
      var wide = availableSpace >= grid.minWideWidth;
      var numRowTiles = wide ? grid.maxColCount : grid.minColCount;

      var effectiveGridWidth = wide ?
          Math.min(Math.max(availableSpace, grid.minWideWidth),
                   grid.maxWideWidth) :
          grid.narrowWidth;
      var realTileValues = tileValuesForGrid(effectiveGridWidth, numRowTiles);

      // leftMargin centers the grid within the avaiable space.
      var minMargin = wide ? MIN_WIDE_MARGIN : 0;
      var leftMargin =
          Math.max(minMargin,
                   (this.tileGrid_.clientWidth - effectiveGridWidth) / 2);
      return {
        numRowTiles: numRowTiles,
        leftMargin: leftMargin,
        colWidth: realTileValues.offsetX,
        rowHeight: this.heightForWidth(realTileValues.tileWidth) +
            realTileValues.interTileSpacing,
        tileWidth: realTileValues.tileWidth,
        wide: wide,
      };
    },

    /**
     * Calculates the x/y coordinates for an element and moves it there.
     * @param {number} index The index of the element to be positioned.
     * @param {number} indexOffset If provided, this is added to |index| when
     *     positioning the tile. The effect is that the tile will be positioned
     *     in a non-default location.
     * @private
     */
    positionTile_: function(index, indexOffset) {
      var grid = this.gridValues_;
      var layout = this.calculateLayoutValues_();

      indexOffset = typeof indexOffset != 'undefined' ? indexOffset : 0;
      // Add the offset _after_ the modulus division. We might want to show the
      // tile off the side of the grid.
      var col = index % layout.numRowTiles + indexOffset;
      var row = Math.floor(index / layout.numRowTiles);
      // Calculate the final on-screen position for the tile.
      var realX = col * layout.colWidth + layout.leftMargin;
      var realY = row * layout.rowHeight;

      // Calculate the portion of the tile's position that should be animated.
      var animatedTileValues = layout.wide ?
          grid.wideTileValues : grid.narrowTileValues;
      // Animate the difference between three-wide and six-wide.
      var animatedLeftMargin = layout.wide ?
          0 : (grid.minWideWidth - MIN_WIDE_MARGIN - grid.narrowWidth) / 2;
      var animatedX = col * animatedTileValues.offsetX + animatedLeftMargin;
      var animatedY = row * (this.heightForWidth(animatedTileValues.tileWidth) +
                             animatedTileValues.interTileSpacing);

      var tile = this.tileElements_[index];
      tile.setGridPosition(animatedX, animatedY);
      tile.firstChild.setBounds(layout.tileWidth,
                                realX - animatedX,
                                realY - animatedY);

      // This code calculates whether the tile needs to show a clone of itself
      // wrapped around the other side of the tile grid.
      var offTheRight = col - indexOffset == layout.numRowTiles - 1;
      var offTheLeft = col - indexOffset == 0;
      if (this.dragEnters_ > 0 && (offTheRight || offTheLeft)) {
        var sign = offTheRight ? 1 : -1;
        tile.showDoppleganger(-layout.numRowTiles * layout.colWidth * sign,
                              layout.rowHeight * sign);
      } else {
        tile.clearDoppleganger();
      }
    },

    /**
     * Gets the index of the tile that should occupy coordinate (x, y). Note
     * that this function doesn't care where the tiles actually are, and will
     * return an index even for the space between two tiles. This function is
     * effectively the inverse of |positionTile_|.
     * @param {number} x The x coordinate, in pixels, relative to the top left
     *     of tileGrid_.
     * @param {number} y The y coordinate.
     * @private
     */
    getWouldBeIndexForPoint_: function(x, y) {
      var grid = this.gridValues_;
      var layout = this.calculateLayoutValues_();

      var col = Math.floor((x - layout.leftMargin) / layout.colWidth);
      if (col < 0 || col >= layout.numRowTiles)
        return -1;

      var row = Math.floor((y - this.tileGrid_.offsetTop) / layout.rowHeight);
      return row * layout.numRowTiles + col;
    },

    /**
     * Window resize event handler. Window resizes may trigger re-layouts.
     * @param {Object} e The resize event.
     */
    onResize_: function(e) {
      // Do nothing if the width didn't change.
      if (this.maskWidth_ == this.clientWidth)
        return;

      this.updateMask_();
      this.classList.add('animating-tile-page');

      for (var i = 0; i < this.tileElements_.length; i++) {
        this.positionTile_(i);
      }
    },

    /**
     * The tile grid has an image mask which fades at the edges. This is only
     * noticeable when doppleganger tiles are entering or exiting the grid.
     */
    updateMask_: function() {
      if (this.clientWidth == this.maskWidth_)
        return;
      this.maskWidth_ = this.clientWidth;

      var leftMargin = this.calculateLayoutValues_().leftMargin;
      var fadeDistance = 20;
      var gradient =
          '-webkit-linear-gradient(left,' +
              'transparent, ' +
              'transparent ' + (leftMargin - fadeDistance) + 'px, ' +
              'black ' + leftMargin + 'px, ' +
              'black ' + (this.clientWidth - leftMargin) + 'px, ' +
              'transparent ' + (this.clientWidth - leftMargin + fadeDistance) +
                  'px, ' +
              'transparent)';
      this.style.WebkitMaskBoxImage = gradient;
    },

    /**
     * Get the height for a tile of a certain width. Override this function to
     * get non-square tiles.
     * @param {number} width The pixel width of a tile.
     * @return {number} The height for |width|.
     */
    heightForWidth: function(width) {
      return width;
    },

    /** Dragging **/

    /**
     * The number of un-paired dragenter events that have fired on |this|. This
     * is incremented by |onDragEnter_| and decremented by |onDragLeave_|. This
     * is necessary because dragging over child widgets will fire additional
     * enter and leave events on |this|.
     * @type {number}
     * @private
     */
    dragEnters_: 0,

    /**
     * Handler for dragenter events fired on |tileGrid_|.
     * @param {Event} e A MouseEvent for the drag.
     * @private
     */
    onDragEnter_: function(e) {
      if (++this.dragEnters_ > 1)
        return;

      this.classList.add('animating-tile-page');
      this.dragItemIndex_ = TilePage.currentlyDraggingTile.index;
      this.currentDropIndex_ = this.dragItemIndex_;
    },

    /**
     * Handler for dragover events fired on |tileGrid_|.
     * @param {Event} e A MouseEvent for the drag.
     * @private
     */
    onDragOver_: function(e) {
      e.dataTransfer.dropEffect = 'move';
      var draggedTile = TilePage.currentlyDraggingTile;
      if (!draggedTile)
        return;

      e.preventDefault();

      var newDragIndex = this.getWouldBeIndexForPoint_(e.clientX, e.clientY);
      if (newDragIndex < 0 || newDragIndex >= this.tileElements_.length)
        newDragIndex = this.dragItemIndex_;
      this.updateDropIndicator_(newDragIndex);
    },

    /**
     * Handler for drop events fired on |tileGrid_|.
     * @param {Event} e A MouseEvent for the drag.
     * @private
     */
    onDrop_: function(e) {
      this.dragEnters_ = 0;
      e.stopPropagation();

      var index = this.currentDropIndex_;
      if (index == this.dragItemIndex_)
        return;

      var adjustment = index > this.dragItemIndex_ ? 1 : 0;
      this.tileGrid_.insertBefore(
          TilePage.currentlyDraggingTile,
          this.tileElements_[this.currentDropIndex_ + adjustment]);
      this.cleanUpDrag_();
    },

    /**
     * Handler for dragleave events fired on |tileGrid_|.
     * @param {Event} e A MouseEvent for the drag.
     * @private
     */
    onDragLeave_: function(e) {
      if (--this.dragEnters_ > 0)
        return;

      this.cleanUpDrag_();
    },

    /**
     * Makes sure all the tiles are in the right place after a drag is over.
     * @private
     */
    cleanUpDrag_: function() {
      for (var i = 0; i < this.tileElements_.length; i++) {
        // The current drag tile will be positioned in its dragend handler.
        if (this.tileElements_[i] == this.currentlyDraggingTile)
          continue;
        this.positionTile_(i);
      }
      this.classList.remove('animating-tile-page');
    },

    /**
     * Updates the visual indicator for the drop location for the active drag.
     * @param {Event} e A MouseEvent for the drag.
     * @private
     */
    updateDropIndicator_: function(newDragIndex) {
      var oldDragIndex = this.currentDropIndex_;
      if (newDragIndex == oldDragIndex)
        return;

      var repositionStart = Math.min(newDragIndex, oldDragIndex);
      var repositionEnd = Math.max(newDragIndex, oldDragIndex);

      for (var i = repositionStart; i <= repositionEnd; i++) {
        if (i == this.dragItemIndex_)
          continue;
        else if (i > this.dragItemIndex_)
          var adjustment = i <= newDragIndex ? -1 : 0;
        else
          var adjustment = i >= newDragIndex ? 1 : 0;

        this.positionTile_(i, adjustment);
      }
      this.currentDropIndex_ = newDragIndex;
    },
  };

  return {
    TilePage: TilePage,
  };
});
