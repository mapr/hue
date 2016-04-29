'use strict';

module.exports = function (grunt) {

  grunt.initConfig({
    jasmine: {
      hue: {
        src: [
          'desktop/core/src/desktop/static/desktop/ext/js/jquery/jquery-2.1.1.min.js',
          'desktop/core/src/desktop/static/desktop/ext/js/require.js',
          'desktop/core/src/desktop/static/desktop/js/hdfsAutocompleter.js',
        ],
        options: {
          specs: 'desktop/core/src/desktop/static/desktop/spec/*Spec.js',
        }
      }
    }
  });

  grunt.loadNpmTasks('grunt-contrib-jasmine');

  grunt.registerTask('default', ['jasmine']);

};