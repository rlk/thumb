#!/bin/sh

slogin vellum8-10 'cd src/thumb; nohup ./thumb vellum8-2 &' &
slogin vellum8-10 'cd src/thumb; nohup ./thumb vellum8-1 &' &
